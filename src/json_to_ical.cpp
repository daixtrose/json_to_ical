#include "json_to_ical.h"
#include <random>
#include <chrono>
#include <algorithm>

// JSON to CalendarEvent conversion
void from_json(const json& j, CalendarEvent& event) {
    j.at("summary").get_to(event.summary);

    if (j.contains("description")) {
        j.at("description").get_to(event.description);
    }

    if (j.contains("location")) {
        j.at("location").get_to(event.location);
    }

    j.at("start_datetime").get_to(event.start_datetime);
    j.at("end_datetime").get_to(event.end_datetime);

    if (j.contains("timezone")) {
        j.at("timezone").get_to(event.timezone);
    } else {
        event.timezone = "UTC";
    }

    if (j.contains("uid")) {
        j.at("uid").get_to(event.uid);
    }

    if (j.contains("organizer_name")) {
        j.at("organizer_name").get_to(event.organizer_name);
    }

    if (j.contains("organizer_email")) {
        j.at("organizer_email").get_to(event.organizer_email);
    }

    if (j.contains("attendees")) {
        j.at("attendees").get_to(event.attendees);
    }

    if (j.contains("all_day_event")) {
        j.at("all_day_event").get_to(event.all_day_event);
    }
}

ICalGenerator::ICalGenerator() 
    : prodid_("-//JSON to iCal Converter//EN") {
}

void ICalGenerator::addEvent(const CalendarEvent& event) {
    events_.push_back(event);
}

bool ICalGenerator::loadFromJson(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        json j;
        file >> j;

        // Support both single event and array of events
        if (j.is_array()) {
            for (const auto& event_json : j) {
                CalendarEvent event;
                from_json(event_json, event);

                // Generate UID if not provided
                if (event.uid.empty()) {
                    event.uid = generateUID();
                }

                events_.push_back(event);
            }
        } else {
            CalendarEvent event;
            from_json(j, event);

            if (event.uid.empty()) {
                event.uid = generateUID();
            }

            events_.push_back(event);
        }

        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

std::string ICalGenerator::toICalendar() const {
    std::ostringstream oss;

    // Calendar header
    oss << "BEGIN:VCALENDAR\r\n";
    oss << "VERSION:2.0\r\n";
    oss << "PRODID:" << prodid_ << "\r\n";
    oss << "CALSCALE:GREGORIAN\r\n";
    oss << "METHOD:PUBLISH\r\n";

    // Add each event
    for (const auto& event : events_) {
        oss << "BEGIN:VEVENT\r\n";

        // UID
        oss << "UID:" << event.uid << "\r\n";

        // Summary (title)
        oss << "SUMMARY:" << escapeICalText(event.summary) << "\r\n";

        // Description
        if (!event.description.empty()) {
            oss << "DESCRIPTION:" << escapeICalText(event.description) << "\r\n";
        }

        // Location
        if (!event.location.empty()) {
            oss << "LOCATION:" << escapeICalText(event.location) << "\r\n";
        }

        // Start and end times
        if (event.all_day_event) {
            oss << "DTSTART;VALUE=DATE:" << formatDateTime(event.start_datetime, true) << "\r\n";
            oss << "DTEND;VALUE=DATE:" << formatDateTime(event.end_datetime, true) << "\r\n";
        } else {
            oss << "DTSTART;TZID=" << event.timezone << ":" 
                << formatDateTime(event.start_datetime) << "\r\n";
            oss << "DTEND;TZID=" << event.timezone << ":" 
                << formatDateTime(event.end_datetime) << "\r\n";
        }

        // Organizer
        if (!event.organizer_email.empty()) {
            oss << "ORGANIZER;CN=";
            if (!event.organizer_name.empty()) {
                oss << escapeICalText(event.organizer_name);
            } else {
                oss << event.organizer_email;
            }
            oss << ":MAILTO:" << event.organizer_email << "\r\n";
        }

        // Attendees
        for (const auto& attendee : event.attendees) {
            oss << "ATTENDEE;ROLE=REQ-PARTICIPANT;PARTSTAT=NEEDS-ACTION;RSVP=TRUE:"
                << "MAILTO:" << attendee << "\r\n";
        }

        // Timestamp
        oss << "DTSTAMP:" << getCurrentTimestamp() << "\r\n";

        // Status
        oss << "STATUS:CONFIRMED\r\n";

        oss << "END:VEVENT\r\n";
    }

    // Calendar footer
    oss << "END:VCALENDAR\r\n";

    return oss.str();
}

bool ICalGenerator::saveToFile(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create file " << filename << std::endl;
            return false;
        }

        file << toICalendar();

        std::cout << "Successfully saved to " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing file: " << e.what() << std::endl;
        return false;
    }
}

std::string ICalGenerator::generateUID() const {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);

    std::ostringstream oss;
    oss << timestamp << "-" << dis(gen) << "@json-to-ical";
    return oss.str();
}

std::string ICalGenerator::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::gmtime(&now_c);

    std::ostringstream oss;
    oss << std::put_time(now_tm, "%Y%m%dT%H%M%SZ");
    return oss.str();
}

std::string ICalGenerator::formatDateTime(const std::string& datetime, bool isAllDay) const {
    // Remove any separators if present (support YYYY-MM-DDTHH:MM:SS format)
    std::string result = datetime;
    result.erase(std::remove(result.begin(), result.end(), '-'), result.end());
    result.erase(std::remove(result.begin(), result.end(), ':'), result.end());

    if (isAllDay) {
        // All-day events only need YYYYMMDD
        return result.substr(0, 8);
    }

    return result;
}

std::string ICalGenerator::escapeICalText(const std::string& text) const {
    std::string result;
    result.reserve(text.size());

    for (char c : text) {
        switch (c) {
            case '\\':
                result += "\\\\";
                break;
            case ';':
                result += "\\;";
                break;
            case ',':
                result += "\\,";
                break;
            case '\n':
                result += "\\n";
                break;
            default:
                result += c;
        }
    }

    return result;
}
