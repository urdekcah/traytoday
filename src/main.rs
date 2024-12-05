use anyhow::Result;
use clap::{Parser, Subcommand};
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
}

#[tokio::main]
async fn main() -> Result<()> {
  let args = Cli::parse();
  let client = NeisClient::new();

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
    None => {
      println!("No command provided");
    }
  }

  Ok(())
}
