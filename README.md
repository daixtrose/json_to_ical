# JSON to iCalendar Converter

A C++ command-line tool that converts JSON files containing calendar event data to iCalendar (.ics) format.

## Features

- Parse JSON files with single or multiple calendar events
- Support for all-day events
- Timezone support
- Organizer and attendee information
- Automatic UID generation
- RFC 5545 compliant iCalendar output
- Proper text escaping for iCalendar format

## Dependencies

- C++11 or later
- nlohmann/json library (header-only)

## Building

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

### Manual compilation with g++

If you have nlohmann/json.hpp in your include path:

```bash
g++ -std=c++11 -I./include -o json_to_ical src/main.cpp src/json_to_ical.cpp
```

## Installing nlohmann/json

### Debian/Ubuntu
```bash
sudo apt-get install nlohmann-json3-dev
```

### macOS (Homebrew)
```bash
brew install nlohmann-json
```

### Arch Linux
```bash
sudo pacman -S nlohmann-json
```

### Manually (header-only)
```bash
wget https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
mkdir -p /usr/local/include/nlohmann
sudo mv json.hpp /usr/local/include/nlohmann/
```

## Usage

```bash
./json_to_ical input.json [output.ics]
```

If output file is not specified, `output.ics` will be used by default.

## JSON Format

### Single Event

```json
{
  "summary": "Team Meeting",
  "description": "Quarterly planning meeting",
  "location": "Conference Room A",
  "start_datetime": "20250120T140000",
  "end_datetime": "20250120T160000",
  "timezone": "America/New_York",
  "organizer_name": "John Doe",
  "organizer_email": "[email protected]",
  "attendees": [
    "[email protected]",
    "[email protected]"
  ]
}
```

### Multiple Events

```json
[
  {
    "summary": "First Event",
    "start_datetime": "20250120T140000",
    "end_datetime": "20250120T160000"
  },
  {
    "summary": "Second Event",
    "start_datetime": "20250121T100000",
    "end_datetime": "20250121T110000",
    "all_day_event": false
  }
]
```

### All-Day Event

```json
{
  "summary": "Company Holiday",
  "description": "New Year Holiday",
  "start_datetime": "20250101",
  "end_datetime": "20250102",
  "all_day_event": true
}
```

## JSON Fields

### Required Fields
- `summary`: Event title/summary
- `start_datetime`: Start date/time in format YYYYMMDDTHHMMSS or YYYYMMDD for all-day events
- `end_datetime`: End date/time in format YYYYMMDDTHHMMSS or YYYYMMDD for all-day events

### Optional Fields
- `description`: Event description
- `location`: Event location
- `timezone`: Timezone identifier (default: "UTC")
- `uid`: Unique identifier (auto-generated if not provided)
- `organizer_name`: Organizer's display name
- `organizer_email`: Organizer's email address
- `attendees`: Array of attendee email addresses
- `all_day_event`: Boolean flag for all-day events (default: false)

## Date/Time Formats

- Regular events: `YYYYMMDDTHHMMSS` (e.g., "20250120T140000" for Jan 20, 2025, 2:00 PM)
- All-day events: `YYYYMMDD` (e.g., "20250120" for Jan 20, 2025)
- Alternative format with separators is also supported: `YYYY-MM-DDTHH:MM:SS`

## Output

The tool generates an iCalendar (.ics) file that can be imported into:
- Google Calendar
- Apple Calendar
- Microsoft Outlook
- Thunderbird
- Any other RFC 5545 compliant calendar application

## Example

```bash
# Convert single event
./json_to_ical examples/single_event.json team_meeting.ics

# Convert multiple events
./json_to_ical examples/multiple_events.json calendar.ics
```

## License

This tool is provided as example code for educational purposes.
