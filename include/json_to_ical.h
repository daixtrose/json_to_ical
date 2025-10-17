#ifndef JSON_TO_ICAL_H
#define JSON_TO_ICAL_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <libical/ical.h>

using json = nlohmann::json;

// Event structure to hold calendar event data
struct CalendarEvent {
    std::string summary;
    std::string description;
    std::string location;
    std::string start_datetime;  // ISO 8601 format: YYYYMMDDTHHMMSS
    std::string end_datetime;    // ISO 8601 format: YYYYMMDDTHHMMSS
    std::string timezone;        // e.g., "America/New_York"
    std::string uid;             // Unique identifier
    std::string organizer_name;
    std::string organizer_email;
    std::vector<std::string> attendees; // Email addresses
    bool all_day_event = false;
};

// JSON to CalendarEvent conversion functions
void from_json(const json& j, CalendarEvent& event);

// iCalendar generator class using libical
class ICalGenerator {
public:
    ICalGenerator();
    ~ICalGenerator();

    // Add event from CalendarEvent struct
    void addEvent(const CalendarEvent& event);

    // Load events from JSON file
    bool loadFromJson(const std::string& filename);

    // Export to iCalendar format using libical
    std::string toICalendar() const;

    // Save to .ics file
    bool saveToFile(const std::string& filename) const;

private:
    icalcomponent* vcalendar_;
    std::string prodid_;

    // Helper functions
    std::string generateUID() const;
    icaltimetype parseDateTime(const std::string& datetime, bool isAllDay = false) const;
    icalcomponent* createVEvent(const CalendarEvent& event) const;
};

#endif // JSON_TO_ICAL_H
