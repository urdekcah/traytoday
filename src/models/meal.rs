use serde::{Deserialize, Serialize};
use std::fmt;

#[derive(Debug, Serialize, Deserialize)]
#[serde(rename_all = "SCREAMING_SNAKE_CASE")]
pub struct Meal {
  pub atpt_ofcdc_sc_code: String,
  pub atpt_ofcdc_sc_nm: String,
  pub sd_schul_code: String,
  pub schul_nm: String,
  pub mmeal_sc_code: String,
  pub mmeal_sc_nm: String,
  pub mlsv_ymd: String,
  pub mlsv_fgr: f64,
  pub ddish_nm: String,
  pub orplc_info: String,
  pub cal_info: String,
  pub ntr_info: String,
  pub mlsv_from_ymd: String,
  pub mlsv_to_ymd: String,
  pub load_dtm: String,
}

impl fmt::Display for Meal {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
    write!(f, "   {}", self.ddish_nm.replace("<br/>", "\n   "))
  }
}
