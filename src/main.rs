use anyhow::Result;
use clap::{Parser, Subcommand};
use std::fmt;
use std::io::Write;
use termcolor::{Color, ColorChoice, ColorSpec, StandardStream, WriteColor};
use traytoday::client::NeisClient;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
  #[command(subcommand)]
  command: Option<Commands>,
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
}

#[tokio::main]
async fn main() -> Result<()> {
  let args = Cli::parse();
  let config = traytoday::config::Config::load()?;
  let client = NeisClient::new(&config);

  match args.command {
    Some(Commands::Search { name }) => {
      let schools = client.search_schools(&name).await?;
      for (i, school) in schools.iter().enumerate() {
        println!("============== {} ==============", school.schul_nm);
        println!("{}", school);
        if i >= schools.len() - 1 {
          println!("===============================================");
        }
      }
    }
    Some(Commands::Set { name }) => {
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
    }
    Some(Commands::SetKey { key }) => {
      let mut new_config = config.clone();
      new_config.api_key = Some(key);
      new_config.save()?;
      println!("API key set");
    }
    None => {
      if config.edu_code.is_empty() || config.school_code.is_empty() {
        println!("No school set. Use `traytoday set` to set your school.");
      } else {
        let date = chrono::Local::now().format("%Y%m%d").to_string();
        let meals = client
          .get_meals_for_dates(&config.edu_code, &config.school_code, &[date])
          .await?;
        if meals[0].is_empty() {
          println!("No meals found for today.");
        }
        let mut stdout = StandardStream::stdout(ColorChoice::Always);
        for (i, meal) in meals[0].iter().enumerate() {
          stdout
            .set_color(ColorSpec::new().set_bold(true).set_fg(Some(Color::White)))
            .map_err(|_| fmt::Error)?;
          write!(&mut stdout, "{}. ", i + 1)?;
          stdout
            .set_color(ColorSpec::new().set_bold(true).set_fg(Some(Color::Yellow)))
            .map_err(|_| fmt::Error)?;
          writeln!(&mut stdout, "{}", meal.mmeal_sc_nm)?;
          stdout.reset().map_err(|_| fmt::Error)?;
          writeln!(&mut stdout, "{}", meal)?;
        }
      }
    }
  }

  Ok(())
}
