# TrayToday
TrayToday is a sleek, Rust-based application designed to quickly and effortlessly retrieve school meal information using the NEIS Meal API.

## Installation
TrayToday is developed in Rust, so you'll need Rust installed on your system. To install Rust, visit the [official Rust website](https://www.rust-lang.org/tools/install). Once Rust is set up, you can install TrayToday with the following command:
```bash  
cargo install traytoday  
```  

## Usage
### Primary Commands
- **`traytoday search <school name>`**: Conduct an erudite search for educational institutions that match the specified name. Note: Without an API key, results are capped at five entries.
- **`traytoday set <school name>`**: Search for a school, select from the enumerated results, and designate it as your default institution. As with the search command, the absence of an API key limits results to five options.
- **`traytoday set-key <api key>`**: Set your API key. If you set an incorrect key, an error will occur when using other command.
- **`traytoday date <date>`**: Retrieve the meal information for a specific date. (e.g. `traytoday date 2025-02-08`, `traytoday date tomorrow`, `traytoday date week`, `traytoday date mon`)
- **`traytoday`**: Retrieve the meal information for your currently configured school when no additional arguments are provided.

### Flags
- **`--allergen <ingredient>`**: Specify the allergenic component to monitor. For example, `traytoday --allergen peanut` will trigger a notification if any meal contains peanuts. (only in: `root`, **`date`**) [See all available allergens](doc/reference/allergens.md)

### Examples
below is an example of how to use TrayToday:
```bash
$ traytoday set 선린인터넷
1. 선린인터넷고등학교 (서울특별시교육청)
- 서울특별시 용산구 원효로97길 33-4 (청파동3가)
> 1
School set to 선린인터넷고등학교
$ traytoday
1. 중식
   기장밥
   스팸짜글이찌개 (1.2.5.6.9.10.13.15.16)
   연두부계란찜 (1.5.9)
   통새우만두,초간장(*) (1.5.6.9.10.15.16.17.18)
   배추김치 (9)
   초코스틱(*) (2.5)
   단감
```

## Contact
For inquiries or bug reports, please use the [Issues section](https://github.com/urdekcah/traytoday/issues) of the project.