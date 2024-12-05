use crate::models::School;
use crate::{Error, Result};
use reqwest::Client;
use serde_json::Value;
use url::Url;

const API_BASE_URL: &str = "https://open.neis.go.kr";
const SCHOOL_INFO_ENDPOINT: &str = "/hub/schoolInfo";

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

  fn build_school_url(&self) -> Result<Url> {
    let mut url =
      Url::parse(API_BASE_URL).map_err(|_| Error::ConfigError("Invalid API URL".to_string()))?;
    url.set_path(SCHOOL_INFO_ENDPOINT);
    url.query_pairs_mut().append_pair("Type", "json");
    Ok(url)
  }
}

impl Default for NeisClient {
  fn default() -> Self {
    Self::new()
  }
}
