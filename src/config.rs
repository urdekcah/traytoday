use serde::{Deserialize, Serialize};
use std::{fs, path::PathBuf};
use crate::{Error, Result};

#[derive(Debug, Serialize, Deserialize, Clone, PartialEq)]
pub struct Config {
  #[serde(default = "default_language")]
  pub language: String,
  #[serde(default)]
  pub edu_code: String,
  #[serde(default)]
  pub school_code: String,
}

fn default_language() -> String {
  "en".to_string()
}

impl Config {
  const CONFIG_DIR: &'static str = ".fishydino/.traytoday";
  const CONFIG_FILE: &'static str = "config.toml";

  pub fn new() -> Self {
    Self::default()
  }

  pub fn load() -> Result<Self> {
    let config_path = Self::get_config_path()?;

    if !config_path.exists() {
      return Ok(Self::default());
    }

    let content = fs::read_to_string(&config_path)?;
    Ok(toml::from_str(&content)?)
  }

  pub fn save(&self) -> Result<()> {
    let config_path = Self::get_config_path()?;

    if let Some(parent) = config_path.parent() {
      fs::create_dir_all(parent)?;
    }

    let content = toml::to_string_pretty(self)?;
    fs::write(config_path, content)?;
    Ok(())
  }

  fn get_config_path() -> Result<PathBuf> {
    let home = home::home_dir().ok_or(Error::HomeError)?;
    Ok(home.join(Self::CONFIG_DIR).join(Self::CONFIG_FILE))
  }

  pub fn update(&mut self, new_config: Config) {
    self.language = new_config.language;
    self.edu_code = new_config.edu_code;
    self.school_code = new_config.school_code;
  }
}

impl Default for Config {
  fn default() -> Self {
    Self {
      language: default_language(),
      edu_code: String::new(),
      school_code: String::new(),
    }
  }
}