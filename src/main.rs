use anyhow::Result;
use clap::{arg, Args, Parser, Subcommand};
use std::io::Write;
use termcolor::{Color, ColorChoice, ColorSpec, StandardStream, WriteColor};
use traytoday::{
  allergens::AllergenChecker, client::NeisClient, config::Config, models::Meal, utils::date::*,
};

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
  #[command(subcommand)]
  command: Option<Commands>,
  #[arg(long, help = "Allergen to check for")]
  allergen: Option<String>,
}

#[derive(Subcommand)]
enum Commands {
  #[command(about = "Search for schools")]
  Search {
    #[arg(help = "School name to search for")]
    name: String,
  },
  #[command(about = "Set current school")]
  Set {
    #[arg(help = "School name to set")]
    name: String,
  },
  #[command(about = "Set API key")]
  SetKey {
    #[arg(help = "API key to set")]
    key: String,
  },
  #[command(about = "Show meals for specific date(s)")]
  Date(DateCommand),
}

#[derive(Args)]
struct DateCommand {
  #[arg(help = "Date specification (e.g., 2023-10-05, tomorrow, week)")]
  date: String,
  #[arg(long, help = "Allergen to check for")]
  allergen: Option<String>,
}

#[tokio::main]
async fn main() -> Result<()> {
  let args = Cli::parse();
  let config = traytoday::config::Config::load()?;
  let client = NeisClient::new(&config);

  match args.command {
    Some(Commands::Search { name }) => handle_search_command(&client, name).await?,
    Some(Commands::Set { name }) => handle_set_school_command(&client, name).await?,
    Some(Commands::SetKey { key }) => handle_set_key_command(&config, key).await?,
    Some(Commands::Date(cmd)) => handle_date_command(&client, &config, &cmd).await?,
    None => handle_root_command(&client, &args, &config).await?,
  }

  Ok(())
}

async fn handle_search_command(client: &NeisClient, name: String) -> Result<()> {
  let schools = client.search_schools(&name).await?;
  for (i, school) in schools.iter().enumerate() {
    println!("============== {} ==============", school.schul_nm);
    println!("{}", school);
    if i >= schools.len() - 1 {
      println!("===============================================");
    }
  }

  Ok(())
}

async fn handle_set_school_command(client: &NeisClient, name: String) -> Result<()> {
  let schools = client.search_schools(&name).await?;
  let mut stdout = StandardStream::stdout(ColorChoice::Always);

  for (i, school) in schools.iter().enumerate() {
    stdout.set_color(ColorSpec::new().set_bold(true).set_fg(Some(Color::Yellow)))?;
    write!(&mut stdout, "{}. ", i + 1)?;
    stdout.reset()?;
    writeln!(
      &mut stdout,
      "{} ({})",
      school.schul_nm, school.atpt_ofcdc_sc_nm
    )?;

    stdout.set_color(ColorSpec::new().set_bold(true).set_fg(Some(Color::White)))?;
    writeln!(&mut stdout, "- {} {}", school.org_rdnma, school.org_rdnda)?;
    stdout.reset()?;
  }

  print!("> ");
  std::io::stdout().flush()?;
  let mut input = String::new();
  std::io::stdin().read_line(&mut input)?;

  let index: usize = input.trim().parse()?;
  let school = schools
    .get(index - 1)
    .ok_or_else(|| anyhow::anyhow!("Invalid index"))?;

  let config = traytoday::config::Config::load()?;
  let mut new_config = config.clone();
  new_config.edu_code = school.atpt_ofcdc_sc_code.clone();
  new_config.school_code = school.sd_schul_code.clone();
  new_config.save()?;
  println!("School set to {}", school.schul_nm);

  Ok(())
}

async fn handle_set_key_command(config: &Config, key: String) -> Result<()> {
  let mut new_config = config.clone();
  new_config.api_key = Some(key);
  new_config.save()?;
  println!("API key set");

  Ok(())
}

async fn handle_date_command(
  client: &NeisClient,
  config: &Config,
  cmd: &DateCommand,
) -> Result<()> {
  let dates = match cmd.date.to_lowercase().as_str() {
    "week" => get_week_dates(),
    date => vec![parse_date(date)?],
  };
  let checker = AllergenChecker::new();
  let allergens = cmd
    .allergen
    .as_ref()
    .map(|s| s.split(",").map(|a| a.trim()).map(|a| checker.get_number(a).unwrap_or(0)).collect::<Vec<_>>())
    .unwrap_or_else(|| vec![]);

  let meals = client
    .get_meals_for_dates(&config.edu_code, &config.school_code, &dates)
    .await?;

  print_meals(&meals, Some(allergens))?;

  Ok(())
}

async fn handle_root_command(client: &NeisClient, args: &Cli, config: &Config) -> Result<()> {
  if config.edu_code.is_empty() || config.school_code.is_empty() {
    println!("No school set. Use `traytoday set` to set your school.");
  } else {
    let checker = AllergenChecker::new();
    let allergen = args
      .allergen
      .as_ref()
      .map(|s| s.split(",").map(|a| a.trim()).map(|a| checker.get_number(a).unwrap_or(0)).collect::<Vec<_>>())
      .unwrap_or_else(|| vec![]);

    let date = chrono::Local::now().format("%Y%m%d").to_string();
    let meals = client
      .get_meals_for_dates(&config.edu_code, &config.school_code, &[date])
      .await?;
    if meals.is_empty() {
      println!("No meals found for today.");
    }
    print_meals(&meals, Some(allergen))?;
  }

  Ok(())
}

fn print_meals(meals: &[Meal], allergens: Option<Vec<u8>>) -> Result<()> {
  if meals.is_empty() {
    return Ok(());
  }

  let mut stdout = StandardStream::stdout(ColorChoice::Always);
  let mut current_date = String::new();

  for meal in meals {
    if meal.mlsv_ymd != current_date {
      if !current_date.is_empty() {
        writeln!(&mut stdout)?;
      }
      current_date = meal.mlsv_ymd.clone();
      stdout.set_color(ColorSpec::new().set_bold(true).set_fg(Some(Color::Green)))?;
      writeln!(
        &mut stdout,
        "============== {} ==============",
        print_date_pretty(&current_date)?
      )?;
      stdout.reset()?;
    }

    stdout.set_color(ColorSpec::new().set_bold(true).set_fg(Some(Color::White)))?;
    write!(&mut stdout, "{}: ", meal.mmeal_sc_nm)?;
    stdout.reset()?;

    let dish = &meal.ddish_nm;
    if let Some(allergens) = &allergens {
      let marked_dish = dish
        .split("<br/>")
        .map(|item| {
          if allergens.iter().any(|a| item.contains(&format!("{}", a))) {
            format!("\x1b[31m{}\x1b[0m", item)
          } else {
            item.to_string()
          }
        })
        .collect::<Vec<_>>()
        .join("\n   ");

      writeln!(&mut stdout, "\n   {}", marked_dish)?;
    } else {
      writeln!(&mut stdout, "\n   {}", dish.replace("<br/>", "\n   "))?;
    }
  }

  Ok(())
}
