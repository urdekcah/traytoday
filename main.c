#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <json-c/json.h>

#define API_URL_SCHOOLINFO "https://open.neis.go.kr/hub/schoolInfo"
#define API_URL_MEALSERVICE "https://open.neis.go.kr/hub/mealServiceDietInfo"
#define INITIAL_BUFFER_SIZE 1024
#define URL_ENCODE_FACTOR 3

struct SchoolInfo {
  char* ATPT_OFCDC_SC_CODE;
  char* ATPT_OFCDC_SC_NM;
  char* SD_SCHUL_CODE;
  char* SCHUL_NM;
  char* ENG_SCHUL_NM;
  char* SCHUL_KND_SC_NM;
  char* LCTN_SC_NM;
  char* JU_ORG_NM;
  char* FOND_SC_NM;
  char* ORG_RDNZC;
  char* ORG_RDNMA;
  char* ORG_RDNDA;
  char* ORG_TELNO;
  char* HMPG_ADRES;
  char* COEDU_SC_NM;
  char* ORG_FAXNO;
  char* HS_SC_NM;
  char* INDST_SPECL_CCCCL_EXST_YN;
  char* HS_GNRL_BUSNS_SC_NM;
  char* SPCLY_PURPS_HS_ORD_NM;
  char* ENE_BFE_SEHF_SC_NM;
  char* DGHT_SC_NM;
  char* FOND_YMD;
  char* FOAS_MEMRD;
  char* LOAD_DTM;
};

struct MealInfo {
  char* ATPT_OFCDC_SC_CODE;
  char* ATPT_OFCDC_SC_NM;
  char* SD_SCHUL_CODE;
  char* SCHUL_NM;
  char* MMEAL_SC_CODE;
  char* MMEAL_SC_NM;
  char* MLSV_YMD;
  char* MLSV_FGR;
  char* DDISH_NM;
  char* ORPLC_INFO;
  char* CAL_INFO;
  char* NTR_INFO;
  char* MLSV_FROM_YMD;
  char* MLSV_TO_YMD;
  char* LOAD_DTM;
};

typedef struct {
  char* buffer;
  size_t size;
  size_t capacity;
} Buffer;

static Buffer* buffer__new(void) {
  Buffer* buf = malloc(sizeof(Buffer));
  if (!buf) return NULL;
  
  buf->buffer = malloc(INITIAL_BUFFER_SIZE);
  if (!buf->buffer) {
    free(buf);
    return NULL;
  }
  
  buf->buffer[0] = '\0';
  buf->size = 0;
  buf->capacity = INITIAL_BUFFER_SIZE;
  return buf;
}

static void buffer_free(Buffer* buf) {
  if (buf) {
    free(buf->buffer);
    free(buf);
  }
}

static int buffer_resize(Buffer* buf, size_t new_size) {
  size_t new_capacity = buf->capacity;
  while (new_capacity < new_size) {
    new_capacity *= 2;
  }
  
  if (new_capacity != buf->capacity) {
    char* new_buffer = realloc(buf->buffer, new_capacity);
    if (!new_buffer) return -1;
    
    buf->buffer = new_buffer;
    buf->capacity = new_capacity;
  }
  return 0;
}

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
  Buffer* buf = (Buffer*)userp;
  size_t new_size = buf->size + size * nmemb;
  
  if (buffer_resize(buf, new_size + 1) != 0) {
    return 0;
  }
  
  memcpy(buf->buffer + buf->size, contents, size * nmemb);
  buf->size = new_size;
  buf->buffer[buf->size] = '\0';
  
  return size * nmemb;
}

static char* url_encode(const char* str) {
  if (!str) return NULL;
  
  const char hex[] = "0123456789ABCDEF";
  size_t len = strlen(str);
  char* encoded = malloc(len * URL_ENCODE_FACTOR + 1);
  
  if (!encoded) return NULL;
  
  size_t j = 0;
  for (size_t i = 0; i < len; i++) {
    unsigned char c = str[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded[j++] = c;
    } else if (c == ' ') {
      encoded[j++] = '+';
    } else {
      encoded[j++] = '%';
      encoded[j++] = hex[c >> 4];
      encoded[j++] = hex[c & 15];
    }
  }
  encoded[j] = '\0';
  return encoded;
}

static void free_school_info(struct SchoolInfo* school) {
  if (!school) return;
  
  free(school->ATPT_OFCDC_SC_CODE);
  free(school->ATPT_OFCDC_SC_NM);
  free(school->SD_SCHUL_CODE);
  free(school->SCHUL_NM);
  free(school->ENG_SCHUL_NM);
  free(school->SCHUL_KND_SC_NM);
  free(school->LCTN_SC_NM);
  free(school->JU_ORG_NM);
  free(school->FOND_SC_NM);
  free(school->ORG_RDNZC);
  free(school->ORG_RDNMA);
  free(school->ORG_RDNDA);
  free(school->ORG_TELNO);
  free(school->HMPG_ADRES);
  free(school->COEDU_SC_NM);
  free(school->ORG_FAXNO);
  free(school->HS_SC_NM);
  free(school->INDST_SPECL_CCCCL_EXST_YN);
  free(school->HS_GNRL_BUSNS_SC_NM);
  free(school->SPCLY_PURPS_HS_ORD_NM);
  free(school->ENE_BFE_SEHF_SC_NM);
  free(school->DGHT_SC_NM);
  free(school->FOND_YMD);
  free(school->FOAS_MEMRD);
  free(school->LOAD_DTM);
  free(school);
}

static char* get_json_string(struct json_object* obj, const char* key) {
  struct json_object* str_obj = json_object_object_get(obj, key);
  return str_obj ? strdup(json_object_get_string(str_obj)) : NULL;
}

struct SchoolInfo** search_school(const char* school_name) {
  CURL* curl = curl_easy_init();
  if (!curl) return NULL;
  
  Buffer* buf = buffer__new();
  if (!buf) {
    curl_easy_cleanup(curl);
    return NULL;
  }

  char* encoded_name = url_encode(school_name);
  if (!encoded_name) {
    buffer_free(buf);
    curl_easy_cleanup(curl);
    return NULL;
  }

  char url[512];
  snprintf(url, sizeof(url), "%s?Type=json&SCHUL_NM=%s", API_URL_SCHOOLINFO, encoded_name);
  free(encoded_name);

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
  
  struct json_object* school_info = NULL;
  if (!json_object_object_get_ex(root, "schoolInfo", &school_info)) {
    json_object_put(root);
    return NULL;
  }
  
  struct json_object* row_container = json_object_array_get_idx(school_info, 1);
  if (!row_container) {
    json_object_put(root);
    return NULL;
  }
  
  struct json_object* rows = NULL;
  if (!json_object_object_get_ex(row_container, "row", &rows)) {
    json_object_put(root);
    return NULL;
  }
  
  int count = json_object_array_length(rows);
  struct SchoolInfo** schools = calloc(count + 1, sizeof(struct SchoolInfo*));
  if (!schools) {
    json_object_put(root);
    return NULL;
  }
  
  for (int i = 0; i < count; i++) {
    struct json_object* school_obj = json_object_array_get_idx(rows, i);
    struct SchoolInfo* school = calloc(1, sizeof(struct SchoolInfo));
    
    if (!school) {
      for (int j = 0; j < i; j++) {
        free_school_info(schools[j]);
      }
      free(schools);
      json_object_put(root);
      return NULL;
    }
    
    school->ATPT_OFCDC_SC_CODE = get_json_string(school_obj, "ATPT_OFCDC_SC_CODE");
    school->ATPT_OFCDC_SC_NM = get_json_string(school_obj, "ATPT_OFCDC_SC_NM");
    school->SD_SCHUL_CODE = get_json_string(school_obj, "SD_SCHUL_CODE");
    school->SCHUL_NM = get_json_string(school_obj, "SCHUL_NM");
    school->ENG_SCHUL_NM = get_json_string(school_obj, "ENG_SCHUL_NM");
    school->SCHUL_KND_SC_NM = get_json_string(school_obj, "SCHUL_KND_SC_NM");
    school->LCTN_SC_NM = get_json_string(school_obj, "LCTN_SC_NM");
    school->JU_ORG_NM = get_json_string(school_obj, "JU_ORG_NM");
    school->FOND_SC_NM = get_json_string(school_obj, "FOND_SC_NM");
    school->ORG_RDNZC = get_json_string(school_obj, "ORG_RDNZC");
    school->ORG_RDNMA = get_json_string(school_obj, "ORG_RDNMA");
    school->ORG_RDNDA = get_json_string(school_obj, "ORG_RDNDA");
    school->ORG_TELNO = get_json_string(school_obj, "ORG_TELNO");
    school->HMPG_ADRES = get_json_string(school_obj, "HMPG_ADRES");
    school->COEDU_SC_NM = get_json_string(school_obj, "COEDU_SC_NM");
    school->ORG_FAXNO = get_json_string(school_obj, "ORG_FAXNO");
    school->HS_SC_NM = get_json_string(school_obj, "HS_SC_NM");
    school->INDST_SPECL_CCCCL_EXST_YN = get_json_string(school_obj, "INDST_SPECL_CCCCL_EXST_YN");
    school->HS_GNRL_BUSNS_SC_NM = get_json_string(school_obj, "HS_GNRL_BUSNS_SC_NM");
    school->SPCLY_PURPS_HS_ORD_NM = get_json_string(school_obj, "SPCLY_PURPS_HS_ORD_NM");
    school->ENE_BFE_SEHF_SC_NM = get_json_string(school_obj, "ENE_BFE_SEHF_SC_NM");
    school->DGHT_SC_NM = get_json_string(school_obj, "DGHT_SC_NM");
    school->FOND_YMD = get_json_string(school_obj, "FOND_YMD");
    school->FOAS_MEMRD = get_json_string(school_obj, "FOAS_MEMRD");
    school->LOAD_DTM = get_json_string(school_obj, "LOAD_DTM");
    
    schools[i] = school;
  }
  
  schools[count] = NULL;
  json_object_put(root);
  return schools;
}

struct MealInfo** search_meal(const char* atptCode, const char* school_code, const char* date) {
  CURL* curl = curl_easy_init();
  if (!curl) return NULL;
  
  Buffer* buf = buffer__new();
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

  int count = json_object_array_length(rows);
  struct MealInfo** meals = calloc(count + 1, sizeof(struct MealInfo*));
  if (!meals) {
    json_object_put(root);
    return NULL;
  }

  for (int i = 0; i < count; i++) {
    struct json_object* meal_obj = json_object_array_get_idx(rows, i);
    struct MealInfo* meal = calloc(1, sizeof(struct MealInfo));
    
    if (!meal) {
      for (int j = 0; j < i; j++) {
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

int main(int argc, char* argv[]) {
  if (argc != 3 || strcmp(argv[1], "search") != 0) {
    fprintf(stderr, "Usage: %s search <school_name>\n", argv[0]);
    return 1;
  }
  
  struct SchoolInfo** schools = search_school(argv[2]);
  if (!schools) {
    fprintf(stderr, "Failed to find school information\n");
    return 1;
  }
  
  for (int i = 0; schools[i] != NULL; i++) {
    printf("\n=== School #%d Information ===\n", i + 1);
    printf("Regional Education Office Code: %s\n", schools[i]->ATPT_OFCDC_SC_CODE);
    printf("Education Office Code: %s\n", schools[i]->ATPT_OFCDC_SC_NM);
    printf("Name of the Education Office: %s\n", schools[i]->ATPT_OFCDC_SC_NM);
    printf("Administrative Standard Code: %s\n", schools[i]->SD_SCHUL_CODE);
    printf("School Name: %s\n", schools[i]->SCHUL_NM);
    printf("English School Name: %s\n", schools[i]->ENG_SCHUL_NM);
    printf("Type of School: %s\n", schools[i]->SCHUL_KND_SC_NM);
    printf("Province/City Name: %s\n", schools[i]->LCTN_SC_NM);
    printf("Supervisory Organization Name: %s\n", schools[i]->JU_ORG_NM);
    printf("Foundation Name: %s\n", schools[i]->FOND_SC_NM);
    printf("Road Name Postal Code: %s\n", schools[i]->ORG_RDNZC);
    printf("Road Name Address: %s\n", schools[i]->ORG_RDNMA);
    printf("Detailed Road Address: %s\n", schools[i]->ORG_RDNDA);
    printf("Telephone Number: %s\n", schools[i]->ORG_TELNO);
    printf("Homepage Address: %s\n", schools[i]->HMPG_ADRES);
    printf("Coeducational Classification: %s\n", schools[i]->COEDU_SC_NM);
    printf("Fax Number: %s\n", schools[i]->ORG_FAXNO);
    printf("High School Classification: %s\n", schools[i]->HS_SC_NM);
    printf("Existence of Special Classes for Industry: %s\n", schools[i]->INDST_SPECL_CCCCL_EXST_YN);
    printf("High School General Major Classification: %s\n", schools[i]->HS_GNRL_BUSNS_SC_NM);
    printf("Special Purpose High School Series Name: %s\n", schools[i]->SPCLY_PURPS_HS_ORD_NM);
    printf("Entrance Examination Classification: %s\n", schools[i]->ENE_BFE_SEHF_SC_NM);
    printf("Day/Night Division: %s\n", schools[i]->DGHT_SC_NM);
    printf("Establishment Date: %s\n", schools[i]->FOND_YMD);
    printf("School Founding Anniversary: %s\n", schools[i]->FOAS_MEMRD);
    printf("Modification Date: %s\n", schools[i]->LOAD_DTM);
    printf("===============================\n");
    free_school_info(schools[i]);
  }
  
  free(schools);
  return 0;
}