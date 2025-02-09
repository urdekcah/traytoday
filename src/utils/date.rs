use crate::{Error, Result};
use chrono::{Datelike, Duration, Local, Weekday};

pub fn parse_date(date_str: &str) -> Result<String> {
  let today = Local::now().date_naive();

  match date_str.to_lowercase().as_str() {
    "ereyesterday" => Ok((today - Duration::days(2)).format("%Y%m%d").to_string()),
    "yesterday" => Ok((today - Duration::days(1)).format("%Y%m%d").to_string()),
    "today" => Ok(today.format("%Y%m%d").to_string()),
    "tomorrow" => Ok((today + Duration::days(1)).format("%Y%m%d").to_string()),
    "postmorrow" => Ok((today + Duration::days(2)).format("%Y%m%d").to_string()),
    "monday" | "tuesday" | "wednesday" | "thursday" | "friday" => {
      let days_until = match date_str {
        "monday" => 0,
        "tuesday" => 1,
        "wednesday" => 2,
        "thursday" => 3,
        "friday" => 4,
        _ => 0,
      };

      let date = get_week_dates()[days_until].clone();
      Ok(date)
    }
    "mon" | "tue" | "wed" | "thu" | "fri" => {
      let days_until = match date_str {
        "mon" => 0,
        "tue" => 1,
        "wed" => 2,
        "thu" => 3,
        "fri" => 4,
        _ => 0,
      };
      let date = get_week_dates()[days_until].clone();
      Ok(date)
    }
    _ => {
      let date =
        chrono::NaiveDate::parse_from_str(date_str, "%Y-%m-%d").map_err(|e| Error::DateError(e))?;
      Ok(date.format("%Y%m%d").to_string())
    }
  }
}

pub fn get_week_dates() -> Vec<String> {
  let today = Local::now().date_naive();
  let weekday = today.weekday();

  let start = today
    - Duration::days(match weekday {
      Weekday::Mon => 0,
      Weekday::Tue => 1,
      Weekday::Wed => 2,
      Weekday::Thu => 3,
      Weekday::Fri => 4,
      Weekday::Sat => 5,
      Weekday::Sun => 6,
    } as i64);

  (0..5)
    .map(|i| (start + Duration::days(i)).format("%Y%m%d").to_string())
    .collect()
}

pub fn print_date_pretty(date: &str) -> Result<String> {
  let date = chrono::NaiveDate::parse_from_str(date, "%Y%m%d").map_err(|e| Error::DateError(e))?;
  let weekday = date.weekday();
  let formatted_date = date.format("%d.%m.%Y");
  let weekday_str = match weekday {
    Weekday::Mon => "Mon",
    Weekday::Tue => "Tue",
    Weekday::Wed => "Wed",
    Weekday::Thu => "Thu",
    Weekday::Fri => "Fri",
    Weekday::Sat => "Sat",
    Weekday::Sun => "Sun",
  };

  Ok(format!("{} ({})", formatted_date, weekday_str))
}
