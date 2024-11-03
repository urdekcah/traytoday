#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "traytoday.h"
#include "buffer.h"
#include "info/school.h"
#include "utils/url.h"
#include "client.h"

struct SchoolInfo** search_school(const char* school_name) {
  CURL* curl = curl_easy_init();
  if (!curl) return NULL;
  
  struct Buffer* buf = buffer__new();
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
  
  size_t count = json_object_array_length(rows);
  struct SchoolInfo** schools = calloc(count + 1, sizeof(struct SchoolInfo*));
  if (!schools) {
    json_object_put(root);
    return NULL;
  }
  
  for (size_t i = 0; i < count; i++) {
    struct json_object* school_obj = json_object_array_get_idx(rows, i);
    struct SchoolInfo* school = calloc(1, sizeof(struct SchoolInfo));
    
    if (!school) {
      for (size_t j = 0; j < i; j++) {
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

void free_school_info(struct SchoolInfo* school) {
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