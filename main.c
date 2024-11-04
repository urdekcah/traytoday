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
#include "info/school.h"
#include "client.h"

int handle_search_school(char* schulNm) {
  struct SchoolInfo** schools = search_school(schulNm);
  if (!schools) {
    fprintf(stderr, "Failed to find school information\n");
    return 1;
  }
  
  for (int i = 0; schools[i] != NULL; i++) {
    printf("\n=== School #%d Information ===\n", i + 1);
    printf("Regional Education Office Code: %s\n", schools[i]->ATPT_OFCDC_SC_CODE);
    printf("Education Office Code: %s\n", schools[i]->ATPT_OFCDC_SC_NM);
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

int handle_command(int argc, char* argv[]) {
  assert(argc >= 2 && "argc must be greater than or equal to 2");
  if (strcmp(argv[1], "search") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Usage: %s search <school name>\n", argv[0]);
      return 1;
    }
    return handle_search_school(argv[2]);
  }
  
  fprintf(stderr, "Unknown command: %s\n", argv[1]);
  return 1;
}

int main(int argc, char* argv[]) {
  ensure_directory_exist(TRAYTODAY_ROOT_DIR);
  if (argc < 2) {
    struct Version read_version;
    struct Settings read_settings;
    int result = read_setting_file(CONFIG_PATH, &read_version, &read_settings);
    if (result != ERRCODE_OK) {
      fprintf(stderr, "Usage: %s <command> [..args]\n", argv[0]);
      return 1;
    }
  }
  
  return handle_command(argc, argv);
}