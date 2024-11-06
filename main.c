#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "traytoday.h"
#include "error.h"
#include "file.h"
#include "buffer.h"
#include "client.h"
#include "utils/color.h"
#include "utils/time.h"
#include "info/school.h"
#include "info/meal.h"
#include "client.h"

size_t strlcpy(char *dst, const char *src, size_t size) {
  size_t src_len = strlen(src);
  if (size > 0) {
    size_t copy_len = (src_len >= size) ? size - 1 : src_len;
    memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
  }
  return src_len;
}

char* replace(const char* str, const char* find, const char* replace) {
  size_t find_len = strlen(find);
  size_t replace_len = strlen(replace);
  int count = 0;
  const char* tmp = str;
  while ((tmp = strstr(tmp, find)) != NULL) {
    tmp += find_len;
    count++;
  }

  size_t result_len = strlen(str) + (replace_len - find_len) * (size_t)count + 1;
  char* result = (char*)malloc(result_len);
  if (!result) {
    return NULL;
  }

  char* ins;
  char* res_ptr = result;
  while (count--) {
    ins = strstr(str, find);
    size_t len = (size_t)(ins - str);
    memcpy(res_ptr, str, len);
    res_ptr += len;
    memcpy(res_ptr, replace, replace_len);
    res_ptr += replace_len;
    str = ins + find_len;
  }
  strcpy(res_ptr, str);
  return result;
}

int handle_search_school(char* schulNm) {
  struct SchoolInfo** schools = search_school(schulNm);
  if (!schools) {
    fprintf(stderr, "Failed to find school information\n");
    return 1;
  }
  
  for (int i = 0; schools[i] != NULL; i++) {
    printf("\n=== School #%d Information ===\n", i + 1);
    printf("Regional Education Office Code: %s\n", schools[i]->ATPT_OFCDC_SC_CODE);
    printf("Education Office Name: %s\n", schools[i]->ATPT_OFCDC_SC_NM);
    printf("NamSe of the Education Office: %s\n", schools[i]->ATPT_OFCDC_SC_NM);
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

int handle_set_school(char* schulNm) {
  struct SchoolInfo** schools = search_school(schulNm);
  if (!schools) {
    fprintf(stderr, "Failed to find school information\n");
    return 1;
  }

  for (int i = 0; schools[i] != NULL; i++)
     printf(ANSI_COLOR_BOLD_YELLOW "%d. " ANSI_COLOR_RESET "%s (%s)\n  " ANSI_COLOR_BOLD_WHITE "- %s %s" ANSI_COLOR_RESET "\n",
           i + 1,
           schools[i]->SCHUL_NM,
           schools[i]->ATPT_OFCDC_SC_NM,
           schools[i]->ORG_RDNMA,
           schools[i]->ORG_RDNDA);

  printf("> ");
  int school_number;
  if (scanf("%d", &school_number) != 1) {
    fprintf(stderr, "Invalid input\n");
    return 1;
  }

  if (school_number < 1 || schools[school_number - 1] == NULL) {
    fprintf(stderr, "Invalid school number\n");
    return 1;
  }

  struct Settings settings;
  strlcpy(settings.language, "ko", sizeof(settings.language));
  strlcpy(settings.edu_code, schools[school_number - 1]->ATPT_OFCDC_SC_CODE, sizeof(settings.edu_code));
  strlcpy(settings.school_code, schools[school_number - 1]->SD_SCHUL_CODE, sizeof(settings.school_code));

  char* expanded_path = expand_home_directory(CONFIG_PATH);
  int result = write_setting_file(expanded_path, VERSION, settings);
  free(expanded_path);
  if (result != ERRCODE_OK) {
    fprintf(stderr, "Failed to write setting file %d\n", result);
    return 1;
  }

  printf("Successfully set the school\n");
  return 0;
}

int handle_command(int argc, char* argv[]) {
  assert(argc >= 2 && "argc must be greater than or equal to 2");
  if (strcmp(argv[1], "search") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Usage: %s search <school name>\n", argv[0]);
      return 1;
    }
    return handle_search_school(argv[2]);
  } else if (strcmp(argv[1], "set") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Usage: %s set <school name>\n", argv[0]);
      return 1;
    }
    return handle_set_school(argv[2]);
  }
  
  fprintf(stderr, "Unknown command: %s\n", argv[1]);
  return 1;
}

int main(int argc, char* argv[]) {
  ensure_directory_exist(TRAYTODAY_ROOT_DIR);
  if (argc < 2) {
    struct Version read_version;
    struct Settings read_settings;
    char* expanded_path = expand_home_directory(CONFIG_PATH);
    int result = read_setting_file(expanded_path, &read_version, &read_settings);
    free(expanded_path);
    if (result != ERRCODE_OK) {
      fprintf(stderr, "Usage: %s <command> [..args]\n", argv[0]);
      return 1;
    }

    struct SchoolInfo* school = get_school(read_settings.edu_code, read_settings.school_code);
    if (!school) {
      fprintf(stderr, "Failed to get school information\n");
      return 1;
    }

    char time_str[32];
    get_current_time(time_str, sizeof(time_str));
    struct MealInfo** meals = search_meal(school->ATPT_OFCDC_SC_CODE, school->SD_SCHUL_CODE, time_str);

    if (!meals) {
      fprintf(stderr, "Failed to get meal information\n");
      return 1;
    }

    for (int i = 0; meals[i] != NULL; i++) {
      if (i > 0) printf("\n");
      printf("============== %s ==============\n", meals[i]->MMEAL_SC_NM);
      char* meal = replace(meals[i]->DDISH_NM, "<br/>", "\n");
      printf("%s", meal);
      free(meal);
      free_meal_info(meals[i]);
      if (meals[i + 1] == NULL) printf("\n==================================\n");
    }
    return 0;
  }
  
  return handle_command(argc, argv);
}