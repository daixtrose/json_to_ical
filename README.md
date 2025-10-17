# JSON to iCalendar Converter

A C++ tool that converts JSON files to iCalendar (.ics) format using PEGTL for JSON parsing and libical for iCalendar generation. All dependencies are managed via CMake FetchContent.

## Features

- **PEGTL-based JSON parsing**: Uses the PEGTL (Parsing Expression Grammar Template Library) for robust JSON validation
- **libical for iCalendar**: RFC 5545 compliant iCalendar generation
- **Zero manual dependencies**: CMake FetchContent automatically downloads PEGTL and libical
- **Support for all-day events**
- **Timezone support**
- **Organizer and attendee information**
- **Automatic UID generation**

## Dependencies

All dependencies are automatically fetched by CMake:
- C++17 compiler
- CMake >= 3.14
- Git (for fetching dependencies)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

The first build will download PEGTL and libical automatically. Subsequent builds will use the cached versions.

## Usage

```bash
./json_to_ical input.json [output.ics]
```

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
    "summary": "Event 1",
    "start_datetime": "20250120T140000",
    "end_datetime": "20250120T160000"
  },
  {
    "summary": "Event 2",
    "start_datetime": "20250121T100000",
    "end_datetime": "20250121T110000"
  }
]
```

### All-Day Event

```json
{
  "summary": "Company Holiday",
  "start_datetime": "20250101",
  "end_datetime": "20250102",
  "all_day_event": true
}
```

## JSON Fields

### Required
- `summary`: Event title
- `start_datetime`: Format YYYYMMDDTHHMMSS or YYYYMMDD
- `end_datetime`: Format YYYYMMDDTHHMMSS or YYYYMMDD

### Optional
- `description`: Event description
- `location`: Event location
- `timezone`: Timezone identifier (default: "UTC")
- `uid`: Unique identifier (auto-generated if missing)
- `organizer_name`: Organizer display name
- `organizer_email`: Organizer email
- `attendees`: Array of attendee emails
- `all_day_event`: Boolean for all-day events (default: false)

## Implementation Details

### PEGTL Integration
- Uses PEGTL's built-in JSON grammar for validation
- Validates JSON structure before parsing
- Provides detailed error messages for malformed JSON

### libical Integration
- Creates proper VCALENDAR and VEVENT components
- Handles timezone parameters correctly
- Generates RFC 5545 compliant output

### CMake FetchContent
- Automatically downloads and builds dependencies
- No manual installation required
- Dependencies are cached for faster subsequent builds
- Configured to disable unnecessary libical features (docs, tests, examples)

## Example

```bash
# Build the project
mkdir build && cd build
cmake ..
make

# Convert events
./json_to_ical ../examples/single_event.json meeting.ics
./json_to_ical ../examples/multiple_events.json calendar.ics
```

## Troubleshooting

### CMake version too old
```bash
cmake --version  # Should be >= 3.14
```

### Git not found
Make sure git is installed and in your PATH.

### Build errors with C++17
Make sure your compiler supports C++17:
- GCC >= 7
- Clang >= 5
- MSVC >= 19.14

## License

This project uses:
- PEGTL: Boost Software License 1.0
- libical: MPL 2.0 / LGPL 2.1
- This tool: Provided as example code

## Output Compatibility

Generated .ics files work with:
- Google Calendar
- Apple Calendar
- Microsoft Outlook  
- Mozilla Thunderbird
- Any RFC 5545 compliant application
