use thiserror::Error;

pub mod client;
pub mod models;

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
}

pub type Result<T> = std::result::Result<T, Error>;
