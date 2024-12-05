# TrayToday
TrayToday is a sleek, Rust-based application designed to quickly and effortlessly retrieve school meal information using the NEIS Meal API.

## Installation
TrayToday is developed in Rust, so you'll need Rust installed on your system. To install Rust, visit the [official Rust website](https://www.rust-lang.org/tools/install). Once Rust is set up, you can install TrayToday with the following command:
```bash  
cargo install traytoday  
```  

## Usage
TrayToday supports the following commands:
- **`traytoday search <school name>`**: Search for schools by name. If no API key is provided, only up to five results will be displayed.
- **`traytoday set <school name>`**: Search for a school, select one from the results, and set it as your default school. Without an API key, results are limited to five.
- **`traytoday`**: Retrieve the meal information for the currently configured school if no options are specified.

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