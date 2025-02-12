use thiserror::Error;

pub mod allergens;
pub mod client;
pub mod config;
pub mod models;
pub mod utils;

#[derive(Error, Debug)]
pub enum Error {
  #[error("HTTP request failed: {0}")]
  RequestFailed(#[from] reqwest::Error),
  #[error("Failed to parse JSON: {0}")]
  JsonParseFailed(#[from] serde_json::Error),
  #[error("Configuration error: {0}")]
  ConfigError(String),
  #[error("IO error: {0}")]
  IoError(#[from] std::io::Error),
  #[error("Failed to get home directory")]
  HomeError,
  #[error("TOML parsing error: {0}")]
  TomlError(#[from] toml::de::Error),
  #[error("TOML serialization error: {0}")]
  TomlSerError(#[from] toml::ser::Error),
  #[error("API error: {0}")]
  ApiError(String),
  #[error("Date parsing error: {0}")]
  DateError(#[from] chrono::ParseError),
  #[error("Invalid allergen: {0}")]
  AllergenError(String),
}

pub type Result<T> = std::result::Result<T, Error>;
