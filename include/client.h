#ifndef __TRAYTODAY_CLIENT_H__
#define __TRAYTODAY_CLIENT_H__
#include <json-c/json.h>
#include "info/school.h"
#include "info/meal.h"

char* get_json_string(struct json_object* obj, const char* key);
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

struct SchoolInfo** search_school(const char* school_name);
void free_school_info(struct SchoolInfo* school);
struct MealInfo** search_meal(const char* atptCode, const char* school_code, const char* date);
void free_meal_info(struct MealInfo* meal);
#endif // __TRAYTODAY_CLIENT_H__