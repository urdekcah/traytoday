use std::{collections::HashMap, io::Write};
use termcolor::{Color, ColorChoice, ColorSpec, StandardStream, WriteColor};

use crate::Result;
pub const ALLERGENS: &[(u8, &str, &[&str])] = &[
  (1, "난류", &["egg", "eggs"]),
  (2, "우유", &["milk", "dairy"]),
  (3, "메밀", &["buckwheat"]),
  (4, "땅콩", &["peanut", "peanuts"]),
  (5, "대두", &["soy"]),
  (6, "밀", &["wheat"]),
  (7, "고등어", &["mackerel"]),
  (8, "게", &["crab"]),
  (9, "새우", &["shrimp"]),
  (10, "돼지고기", &["pork"]),
  (11, "복숭아", &["peach"]),
  (12, "토마토", &["tomato"]),
  (13, "아황산류", &["sulfite", "sulfites"]),
  (14, "호두", &["walnut"]),
  (15, "닭고기", &["chicken"]),
  (16, "쇠고기", &["beef"]),
  (17, "오징어", &["squid"]),
  (18, "조개류", &["shellfish", "clam", "oyster", "mussel"]),
  (19, "잣", &["pine nut"]),
];

pub struct AllergenChecker {
  allergen_map: HashMap<String, u8>,
}

impl AllergenChecker {
  pub fn new() -> Self {
    let mut allergen_map = HashMap::new();
    for &(num, name, aliases) in ALLERGENS {
      allergen_map.insert(name.to_lowercase(), num);
      for alias in aliases {
        allergen_map.insert(alias.to_lowercase(), num);
      }
    }
    Self { allergen_map }
  }

  pub fn get_number(&self, name: &str) -> Result<u8> {
    let result = self
      .allergen_map
      .get(&name.to_lowercase())
      .copied()
      .unwrap_or(0);
    if result == 0 {
      let mut stderr = StandardStream::stderr(ColorChoice::Always);
      stderr.set_color(ColorSpec::new().set_bold(true).set_fg(Some(Color::Yellow)))?;
      write!(stderr, "warning")?;
      stderr.reset()?;
      write!(stderr, ": Allergen not found ({})\n", name)?;
    }
    Ok(result)
  }
}
