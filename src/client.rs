use crate::config::Config;
use crate::models::{Meal, School};
use crate::{Error, Result};
use reqwest::Client;
use serde_json::Value;
use url::Url;

const API_BASE_URL: &str = "https://open.neis.go.kr";
const SCHOOL_INFO_ENDPOINT: &str = "/hub/schoolInfo";
const MEAL_SERVICE_ENDPOINT: &str = "/hub/mealServiceDietInfo";

pub struct NeisClient {
  client: Client,
  config: Config,
}

impl NeisClient {
  pub fn new(config: &Config) -> Self {
    Self {
      client: Client::new(),
      config: config.clone(),
    }
  }

  pub async fn search_schools(&self, school_name: &str) -> Result<Vec<School>> {
    let mut url = self.build_school_url()?;
    url.query_pairs_mut().append_pair("SCHUL_NM", school_name);

    self.append_api_key(&mut url);
    let response = self.get_json_response(url).await?;

    self.validate_response(&response)?;
    let schools = self.parse_schools(&response).unwrap_or_default();
    Ok(schools)
  }

  pub async fn get_meals_for_dates(
    &self,
    atpt_code: &str,
    school_code: &str,
    dates: &[String],
  ) -> Result<Vec<Vec<Meal>>> {
    let mut all_meals = Vec::new();

    for date in dates {
      let mut url = self.build_meal_url()?;
      url
        .query_pairs_mut()
        .append_pair("ATPT_OFCDC_SC_CODE", atpt_code)
        .append_pair("SD_SCHUL_CODE", school_code)
        .append_pair("MLSV_YMD", date);

      self.append_api_key(&mut url);
      let response = self.get_json_response(url).await?;

      self.validate_response(&response)?;
      let meals = self.parse_meals(&response).unwrap_or_default();
      all_meals.push(meals);
    }

    Ok(all_meals)
  }

  fn build_school_url(&self) -> Result<Url> {
    self.build_base_url(SCHOOL_INFO_ENDPOINT)
  }

  fn build_meal_url(&self) -> Result<Url> {
    self.build_base_url(MEAL_SERVICE_ENDPOINT)
  }

  fn build_base_url(&self, endpoint: &str) -> Result<Url> {
    let mut url =
      Url::parse(API_BASE_URL).map_err(|_| Error::ConfigError("Invalid API URL".to_string()))?;
    url.set_path(endpoint);
    url.query_pairs_mut().append_pair("Type", "json");
    Ok(url)
  }

  fn append_api_key(&self, url: &mut Url) {
    if let Some(api_key) = &self.config.api_key {
      url.query_pairs_mut().append_pair("KEY", api_key);
    }
  }

  async fn get_json_response(&self, url: Url) -> Result<Value> {
    let response = self.client.get(url.as_str()).send().await?.json().await?;
    Ok(response)
  }

  fn validate_response(&self, response: &Value) -> Result<()> {
    if let Some(result) = response["RESULT"].as_object() {
      let code = result["CODE"].as_str().unwrap_or("Unknown error");
      if !["INFO-200", "INFO-000"].contains(&code) {
        return Err(Error::ApiError(format!(
          "API Error: {} - {}",
          code,
          result["MESSAGE"].as_str().unwrap_or("Unknown error")
        )));
      }
    }
    Ok(())
  }

  fn parse_schools(&self, response: &Value) -> Result<Vec<School>> {
    let schools = response["schoolInfo"][1]["row"]
      .as_array()
      .ok_or_else(|| Error::ApiError("I think no schools".to_string()))?
      .iter()
      .filter_map(|school| serde_json::from_value(school.clone()).ok())
      .collect();
    Ok(schools)
  }

  fn parse_meals(&self, response: &Value) -> Result<Vec<Meal>> {
    let meals = response["mealServiceDietInfo"][1]["row"]
      .as_array()
      .ok_or_else(|| Error::ApiError("I think no meals today".to_string()))?
      .iter()
      .filter_map(|meal| serde_json::from_value(meal.clone()).ok())
      .collect();
    Ok(meals)
  }
}
