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
}

impl NeisClient {
  pub fn new() -> Self {
    Self {
      client: Client::new(),
    }
  }

  pub async fn search_schools(&self, school_name: &str) -> Result<Vec<School>> {
    let mut url = self.build_school_url()?;
    url
      .query_pairs_mut()
      .append_pair("SCHUL_NM", school_name)
      .finish();

    let response: Value = self.client.get(url.as_str()).send().await?.json().await?;
    if let Some(result) = response["schoolInfo"][0]["head"][1]["RESULT"].as_object() {
      if result["CODE"] == "INFO-200" {
        return Ok(Vec::new());
      }
    }

    let schools = response["schoolInfo"][1]["row"]
      .as_array()
      .unwrap_or(&vec![])
      .iter()
      .map(|school| serde_json::from_value(school.clone()).unwrap())
      .collect();
    Ok(schools)
  }

  pub async fn get_meals(
    &self,
    atpt_code: &str,
    school_code: &str,
    date: &str,
  ) -> Result<Vec<Meal>> {
    let mut url = self.build_meal_url()?;
    url
      .query_pairs_mut()
      .append_pair("ATPT_OFCDC_SC_CODE", atpt_code)
      .append_pair("SD_SCHUL_CODE", school_code)
      .append_pair("MLSV_YMD", date)
      .finish();

    println!("{}", url.as_str());

    let response: Value = self.client.get(url.as_str()).send().await?.json().await?;
    let meals = response["mealServiceDietInfo"][1]["row"]
      .as_array()
      .unwrap_or(&vec![])
      .iter()
      .map(|meal| serde_json::from_value(meal.clone()).unwrap())
      .collect();
    Ok(meals)
  }

  fn build_school_url(&self) -> Result<Url> {
    let mut url =
      Url::parse(API_BASE_URL).map_err(|_| Error::ConfigError("Invalid API URL".to_string()))?;
    url.set_path(SCHOOL_INFO_ENDPOINT);
    url.query_pairs_mut().append_pair("Type", "json");
    Ok(url)
  }

  fn build_meal_url(&self) -> Result<Url> {
    let mut url =
      Url::parse(API_BASE_URL).map_err(|_| Error::ConfigError("Invalid API URL".to_string()))?;
    url.set_path(MEAL_SERVICE_ENDPOINT);
    url.query_pairs_mut().append_pair("Type", "json");
    Ok(url)
  }
}

impl Default for NeisClient {
  fn default() -> Self {
    Self::new()
  }
}
