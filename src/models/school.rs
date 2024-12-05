use serde::{Deserialize, Serialize};
use std::fmt;
use termcolor::{Color, ColorChoice, ColorSpec, StandardStream, WriteColor};

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

impl fmt::Display for School {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
    let mut stdout = StandardStream::stdout(ColorChoice::Always);

    let fields = [
      ("Regional Education Office Code", &self.atpt_ofcdc_sc_code),
      ("Name of the Education Office", &self.atpt_ofcdc_sc_nm),
      ("Administrative Standard Code", &self.sd_schul_code),
      ("School Name", &self.schul_nm),
      ("English School Name", &self.eng_schul_nm),
      ("Type of School", &self.schul_knd_sc_nm),
      ("Province/City Name", &self.lctn_sc_nm),
      ("Supervisory Organization Name", &self.ju_org_nm),
      ("Foundation Name", &self.fond_sc_nm),
      ("Road Name Postal Code", &self.org_rdnzc),
      ("Road Name Address", &self.org_rdnma),
      ("Detailed Road Address", &self.org_rdnda),
      ("Phone Number", &self.org_telno),
      ("Homepage Address", &self.hmpg_adres),
      ("Coeducational Classification", &self.coedu_sc_nm),
      ("Fax Number", &self.org_faxno),
      ("High School Classification", &self.hs_sc_nm),
      (
        "Existence of Special Classes for Industry",
        &self.indst_specl_ccccl_exst_yn,
      ),
      (
        "High School General Major Classification",
        &self.hs_gnrl_busns_sc_nm,
      ),
      (
        "Special Purpose High School Series Name",
        &self
          .spcly_purps_hs_ord_nm
          .as_deref()
          .unwrap_or("N/A")
          .to_string(),
      ),
      (
        "Entrance Examination Classification",
        &self.ene_bfe_sehf_sc_nm,
      ),
      ("Day/Night Division", &self.dght_sc_nm),
      ("Foundation Date", &self.fond_ymd),
      ("School Founding Anniversary", &self.foas_memrd),
      ("Modification Date", &self.load_dtm),
    ];

    for (i, (label, value)) in fields.iter().enumerate() {
      stdout
        .set_color(ColorSpec::new().set_fg(Some(Color::Yellow)))
        .map_err(|_| fmt::Error)?;
      write!(f, "{:<41}: ", label)?;
      stdout.reset().map_err(|_| fmt::Error)?;
      if i == fields.len() - 1 {
        write!(f, "{}", value)?;
      } else {
        writeln!(f, "{}", value)?;
      }
    }

    Ok(())
  }
}
