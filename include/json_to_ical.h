#ifndef JSON_TO_ICAL_H
#define JSON_TO_ICAL_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <libical/ical.h>
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace peg = tao::pegtl;

// Event structure to hold calendar event data
struct CalendarEvent {
    std::string summary;
    std::string description;
    std::string location;
    std::string start_datetime;
    std::string end_datetime;
    std::string timezone = "UTC";
    std::string uid;
    std::string organizer_name;
    std::string organizer_email;
    std::vector<std::string> attendees;
    bool all_day_event = false;
};

// JSON parser using PEGTL
class JsonParser {
public:
    std::vector<CalendarEvent> parseFile(const std::string& filename);
    std::vector<CalendarEvent> parseString(const std::string& json_str);

private:
    CalendarEvent parseEvent(const std::string& json_str);
    std::vector<CalendarEvent> parseEventArray(const std::string& json_str);
};

// iCalendar generator class using libical
class ICalGenerator {
public:
    ICalGenerator();
    ~ICalGenerator();

    void addEvent(const CalendarEvent& event);
    bool loadFromJson(const std::string& filename);
    std::string toICalendar() const;
    bool saveToFile(const std::string& filename) const;

private:
    icalcomponent* vcalendar_;
    std::string prodid_;
    JsonParser parser_;

    std::string generateUID() const;
    icaltimetype parseDateTime(const std::string& datetime, bool isAllDay = false) const;
    icalcomponent* createVEvent(const CalendarEvent& event) const;
};

#endif // JSON_TO_ICAL_H
