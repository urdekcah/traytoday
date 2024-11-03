#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "traytoday.h"
#include "buffer.h"
#include "info/meal.h"
#include "utils/url.h"
#include "client.h"

struct MealInfo** search_meal(const char* atptCode, const char* school_code, const char* date) {
  CURL* curl = curl_easy_init();
  if (!curl) return NULL;
  
  struct Buffer* buf = buffer__new();
  if (!buf) {
    curl_easy_cleanup(curl);
    return NULL;
  }
  
  char url[512];
  snprintf(url, sizeof(url), "%s?Type=json&ATPT_OFCDC_SC_CODE=%s&SD_SCHUL_CODE=%s&MLSV_YMD=%s",
           API_URL_MEALSERVICE, atptCode, school_code, date);
  
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    buffer_free(buf);
    return NULL;
  }

  struct json_object* root = json_tokener_parse(buf->buffer);
  buffer_free(buf);

  if (!root) return NULL;

  struct json_object* meal_info = NULL;

  if (!json_object_object_get_ex(root, "mealServiceDietInfo", &meal_info)) {
    json_object_put(root);
    return NULL;
  }

  struct json_object* row_container = json_object_array_get_idx(meal_info, 1);
  if (!row_container) {
    json_object_put(root);
    return NULL;
  }

  struct json_object* rows = NULL;
  if (!json_object_object_get_ex(row_container, "row", &rows)) {
    json_object_put(root);
    return NULL;
  }

  size_t count = json_object_array_length(rows);
  struct MealInfo** meals = calloc(count + 1, sizeof(struct MealInfo*));
  if (!meals) {
    json_object_put(root);
    return NULL;
  }

  for (size_t i = 0; i < count; i++) {
    struct json_object* meal_obj = json_object_array_get_idx(rows, i);
    struct MealInfo* meal = calloc(1, sizeof(struct MealInfo));
    
    if (!meal) {
      for (size_t j = 0; j < i; j++) {
        free(meals[j]);
      }
      free(meals);
      json_object_put(root);
      return NULL;
    }
    
    meal->ATPT_OFCDC_SC_CODE = get_json_string(meal_obj, "ATPT_OFCDC_SC_CODE");
    meal->ATPT_OFCDC_SC_NM = get_json_string(meal_obj, "ATPT_OFCDC_SC_NM");
    meal->SD_SCHUL_CODE = get_json_string(meal_obj, "SD_SCHUL_CODE");
    meal->SCHUL_NM = get_json_string(meal_obj, "SCHUL_NM");
    meal->MMEAL_SC_CODE = get_json_string(meal_obj, "MMEAL_SC_CODE");
    meal->MMEAL_SC_NM = get_json_string(meal_obj, "MMEAL_SC_NM");
    meal->MLSV_YMD = get_json_string(meal_obj, "MLSV_YMD");
    meal->MLSV_FGR = get_json_string(meal_obj, "MLSV_FGR");
    meal->DDISH_NM = get_json_string(meal_obj, "DDISH_NM");
    meal->ORPLC_INFO = get_json_string(meal_obj, "ORPLC_INFO");
    meal->CAL_INFO = get_json_string(meal_obj, "CAL_INFO");
    meal->NTR_INFO = get_json_string(meal_obj, "NTR_INFO");
    meal->MLSV_FROM_YMD = get_json_string(meal_obj, "MLSV_FROM_YMD");
    meal->MLSV_TO_YMD = get_json_string(meal_obj, "MLSV_TO_YMD");
    meal->LOAD_DTM = get_json_string(meal_obj, "LOAD_DTM");
    
    meals[i] = meal;
  }

  meals[count] = NULL;
  json_object_put(root);

  return meals;
}

void free_meal_info(struct MealInfo* meal) {
  if (!meal) return;
  
  free(meal->ATPT_OFCDC_SC_CODE);
  free(meal->ATPT_OFCDC_SC_NM);
  free(meal->SD_SCHUL_CODE);
  free(meal->SCHUL_NM);
  free(meal->MMEAL_SC_CODE);
  free(meal->MMEAL_SC_NM);
  free(meal->MLSV_YMD);
  free(meal->MLSV_FGR);
  free(meal->DDISH_NM);
  free(meal->ORPLC_INFO);
  free(meal->CAL_INFO);
  free(meal->NTR_INFO);
  free(meal->MLSV_FROM_YMD);
  free(meal->MLSV_TO_YMD);
  free(meal->LOAD_DTM);
  free(meal);
}