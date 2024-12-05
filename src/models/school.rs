use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "SCREAMING_SNAKE_CASE")]
pub struct School {
  pub atpt_ofcdc_sc_code: String,
  pub atpt_ofcdc_sc_nm: String,
  pub sd_schul_code: String,
  pub schul_nm: String,
  pub eng_schul_nm: String,
  pub schul_knd_sc_nm: String,
  pub lctn_sc_nm: String,
  pub ju_org_nm: String,
  pub fond_sc_nm: String,
  pub org_rdnzc: String,
  pub org_rdnma: String,
  pub org_rdnda: String,
  pub org_telno: String,
  pub hmpg_adres: String,
  pub coedu_sc_nm: String,
  pub org_faxno: String,
  pub hs_sc_nm: String,
  pub indst_specl_ccccl_exst_yn: String,
  pub hs_gnrl_busns_sc_nm: String,
  pub spcly_purps_hs_ord_nm: Option<String>,
  pub ene_bfe_sehf_sc_nm: String,
  pub dght_sc_nm: String,
  pub fond_ymd: String,
  pub foas_memrd: String,
  pub load_dtm: String,
}
