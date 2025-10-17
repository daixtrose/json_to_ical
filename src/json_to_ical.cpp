#include "json_to_ical.h"

#include <random>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <stdexcept>

#include <tao/pegtl/contrib/json.hpp>

// Use PEGTL's contrib JSON grammar
namespace json_grammar = tao::pegtl::json;

// Simple JSON value extractor
class JsonExtractor
{
public:
    static std::string extractString(const std::string &json_str, const std::string &key)
    {
        size_t key_pos = json_str.find("\"" + key + "\"");
        if (key_pos == std::string::npos)
            return "";

        size_t colon_pos = json_str.find(":", key_pos);
        if (colon_pos == std::string::npos)
            return "";

        size_t start_quote = json_str.find("\"", colon_pos);
        if (start_quote == std::string::npos)
            return "";

        size_t end_quote = json_str.find("\"", start_quote + 1);
        if (end_quote == std::string::npos)
            return "";

        return json_str.substr(start_quote + 1, end_quote - start_quote - 1);
    }

    static bool extractBool(const std::string &json_str, const std::string &key)
    {
        size_t key_pos = json_str.find("\"" + key + "\"");
        if (key_pos == std::string::npos)
            return false;

        size_t colon_pos = json_str.find(":", key_pos);
        if (colon_pos == std::string::npos)
            return false;

        size_t true_pos = json_str.find("true", colon_pos);
        size_t false_pos = json_str.find("false", colon_pos);
        size_t comma_pos = json_str.find(",", colon_pos);
        size_t brace_pos = json_str.find("}", colon_pos);

        size_t next_delimiter = std::min(
            comma_pos != std::string::npos ? comma_pos : std::string::npos,
            brace_pos != std::string::npos ? brace_pos : std::string::npos);

        if (true_pos != std::string::npos && true_pos < next_delimiter)
            return true;
        return false;
    }

    static std::vector<std::string> extractStringArray(const std::string &json_str, const std::string &key)
    {
        std::vector<std::string> result;

        size_t key_pos = json_str.find("\"" + key + "\"");
        if (key_pos == std::string::npos)
            return result;

        size_t array_start = json_str.find("[", key_pos);
        if (array_start == std::string::npos)
            return result;

        size_t array_end = json_str.find("]", array_start);
        if (array_end == std::string::npos)
            return result;

        std::string array_content = json_str.substr(array_start + 1, array_end - array_start - 1);

        size_t pos = 0;
        while (pos < array_content.length())
        {
            size_t start_quote = array_content.find("\"", pos);
            if (start_quote == std::string::npos)
                break;

            size_t end_quote = array_content.find("\"", start_quote + 1);
            if (end_quote == std::string::npos)
                break;

            result.push_back(array_content.substr(start_quote + 1, end_quote - start_quote - 1));
            pos = end_quote + 1;
        }

        return result;
    }
};

CalendarEvent JsonParser::parseEvent(const std::string &json_str)
{
    // Validate JSON using PEGTL
    try
    {
        peg::memory_input in(json_str, "json_event");
        if (!peg::parse<json_grammar::text>(in))
        {
            throw std::runtime_error("Invalid JSON format");
        }
    }
    catch (const peg::parse_error &e)
    {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }

    CalendarEvent event;
    event.summary = JsonExtractor::extractString(json_str, "summary");
    event.description = JsonExtractor::extractString(json_str, "description");
    event.location = JsonExtractor::extractString(json_str, "location");
    event.start_datetime = JsonExtractor::extractString(json_str, "start_datetime");
    event.end_datetime = JsonExtractor::extractString(json_str, "end_datetime");

    std::string tz = JsonExtractor::extractString(json_str, "timezone");
    if (!tz.empty())
        event.timezone = tz;

    event.uid = JsonExtractor::extractString(json_str, "uid");
    event.organizer_name = JsonExtractor::extractString(json_str, "organizer_name");
    event.organizer_email = JsonExtractor::extractString(json_str, "organizer_email");
    event.attendees = JsonExtractor::extractStringArray(json_str, "attendees");
    event.all_day_event = JsonExtractor::extractBool(json_str, "all_day_event");

    return event;
}

std::vector<CalendarEvent> JsonParser::parseEventArray(const std::string &json_str)
{
    std::vector<CalendarEvent> events;

    // Validate JSON using PEGTL
    try
    {
        peg::memory_input in(json_str, "json_array");
        if (!peg::parse<json_grammar::text>(in))
        {
            throw std::runtime_error("Invalid JSON format");
        }
    }
    catch (const peg::parse_error &e)
    {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }

    // Simple array element extraction
    size_t pos = 0;
    int brace_count = 0;
    size_t obj_start = 0;
    bool in_object = false;

    for (size_t i = 0; i < json_str.length(); ++i)
    {
        if (json_str[i] == '{')
        {
            if (brace_count == 0)
            {
                obj_start = i;
                in_object = true;
            }
            brace_count++;
        }
        else if (json_str[i] == '}')
        {
            brace_count--;
            if (brace_count == 0 && in_object)
            {
                std::string obj_str = json_str.substr(obj_start, i - obj_start + 1);
                events.push_back(parseEvent(obj_str));
                in_object = false;
            }
        }
    }

    return events;
}

std::vector<CalendarEvent> JsonParser::parseFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    return parseString(content);
}

std::vector<CalendarEvent> JsonParser::parseString(const std::string &json_str)
{
    // Trim whitespace
    size_t start = json_str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        throw std::runtime_error("Empty JSON string");
    }

    if (json_str[start] == '[')
    {
        return parseEventArray(json_str);
    }
    else if (json_str[start] == '{')
    {
        std::vector<CalendarEvent> events;
        events.push_back(parseEvent(json_str));
        return events;
    }

    throw std::runtime_error("JSON must start with [ or {");
}

ICalGenerator::ICalGenerator()
    : prodid_("-//JSON to iCal Converter//EN")
{
    vcalendar_ = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    icalcomponent_add_property(vcalendar_,
                               icalproperty_new_version("2.0"));
    icalcomponent_add_property(vcalendar_,
                               icalproperty_new_prodid(prodid_.c_str()));
    icalcomponent_add_property(vcalendar_,
                               icalproperty_new_calscale("GREGORIAN"));
}

ICalGenerator::~ICalGenerator()
{
    if (vcalendar_)
    {
        icalcomponent_free(vcalendar_);
    }
}

void ICalGenerator::addEvent(const CalendarEvent &event)
{
    icalcomponent *vevent = createVEvent(event);
    if (vevent)
    {
        icalcomponent_add_component(vcalendar_, vevent);
    }
}

bool ICalGenerator::loadFromJson(const std::string &filename)
{
    try
    {
        std::vector<CalendarEvent> events = parser_.parseFile(filename);

        for (auto &event : events)
        {
            if (event.uid.empty())
            {
                event.uid = generateUID();
            }
            addEvent(event);
        }

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

std::string ICalGenerator::toICalendar() const
{
    char *ical_str = icalcomponent_as_ical_string_r(vcalendar_);
    if (!ical_str)
    {
        return "";
    }

    std::string result(ical_str);
    free(ical_str);
    return result;
}

bool ICalGenerator::saveToFile(const std::string &filename) const
{
    try
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not create file " << filename << std::endl;
            return false;
        }

        file << toICalendar();

        std::cout << "Successfully saved to " << filename << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing file: " << e.what() << std::endl;
        return false;
    }
}

std::string ICalGenerator::generateUID() const
{
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch())
                         .count();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);

    std::ostringstream oss;
    oss << timestamp << "-" << dis(gen) << "@json-to-ical";
    return oss.str();
}

icaltimetype ICalGenerator::parseDateTime(const std::string &datetime, bool isAllDay) const
{
    std::string clean_dt = datetime;
    clean_dt.erase(std::remove(clean_dt.begin(), clean_dt.end(), '-'), clean_dt.end());
    clean_dt.erase(std::remove(clean_dt.begin(), clean_dt.end(), ':'), clean_dt.end());

    icaltimetype tt = icaltime_null_time();

    if (isAllDay || clean_dt.length() == 8)
    {
        if (clean_dt.length() >= 8)
        {
            tt.year = std::stoi(clean_dt.substr(0, 4));
            tt.month = std::stoi(clean_dt.substr(4, 2));
            tt.day = std::stoi(clean_dt.substr(6, 2));
            tt.hour = 0;
            tt.minute = 0;
            tt.second = 0;
            tt.is_date = 1;
            // tt.is_utc = 0;
        }
    }
    else if (clean_dt.length() >= 15)
    {
        tt.year = std::stoi(clean_dt.substr(0, 4));
        tt.month = std::stoi(clean_dt.substr(4, 2));
        tt.day = std::stoi(clean_dt.substr(6, 2));
        tt.hour = std::stoi(clean_dt.substr(9, 2));
        tt.minute = std::stoi(clean_dt.substr(11, 2));
        tt.second = std::stoi(clean_dt.substr(13, 2));
        tt.is_date = 0;
        // tt.is_utc = 0;
    }

    return tt;
}

icalcomponent *ICalGenerator::createVEvent(const CalendarEvent &event) const
{
    icalcomponent *vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);

    icalcomponent_add_property(vevent,
                               icalproperty_new_uid(event.uid.c_str()));
    icalcomponent_add_property(vevent,
                               icalproperty_new_summary(event.summary.c_str()));

    if (!event.description.empty())
    {
        icalcomponent_add_property(vevent,
                                   icalproperty_new_description(event.description.c_str()));
    }

    if (!event.location.empty())
    {
        icalcomponent_add_property(vevent,
                                   icalproperty_new_location(event.location.c_str()));
    }

    icaltimetype dtstart = parseDateTime(event.start_datetime, event.all_day_event);
    icalproperty *prop_start = icalproperty_new_dtstart(dtstart);

    if (!event.all_day_event && !event.timezone.empty() && event.timezone != "UTC")
    {
        icalparameter *tzid_param = icalparameter_new_tzid(event.timezone.c_str());
        icalproperty_add_parameter(prop_start, tzid_param);
    }
    icalcomponent_add_property(vevent, prop_start);

    icaltimetype dtend = parseDateTime(event.end_datetime, event.all_day_event);
    icalproperty *prop_end = icalproperty_new_dtend(dtend);

    if (!event.all_day_event && !event.timezone.empty() && event.timezone != "UTC")
    {
        icalparameter *tzid_param = icalparameter_new_tzid(event.timezone.c_str());
        icalproperty_add_parameter(prop_end, tzid_param);
    }
    icalcomponent_add_property(vevent, prop_end);

    icaltimetype dtstamp = icaltime_current_time_with_zone(icaltimezone_get_utc_timezone());
    icalcomponent_add_property(vevent,
                               icalproperty_new_dtstamp(dtstamp));

    if (!event.organizer_email.empty())
    {
        std::string mailto = "mailto:" + event.organizer_email;
        icalproperty *prop_organizer = icalproperty_new_organizer(mailto.c_str());

        if (!event.organizer_name.empty())
        {
            icalparameter *cn_param = icalparameter_new_cn(event.organizer_name.c_str());
            icalproperty_add_parameter(prop_organizer, cn_param);
        }

        icalcomponent_add_property(vevent, prop_organizer);
    }

    for (const auto &attendee_email : event.attendees)
    {
        std::string mailto = "mailto:" + attendee_email;
        icalproperty *prop_attendee = icalproperty_new_attendee(mailto.c_str());

        icalproperty_add_parameter(prop_attendee,
                                   icalparameter_new_role(ICAL_ROLE_REQPARTICIPANT));
        icalproperty_add_parameter(prop_attendee,
                                   icalparameter_new_partstat(ICAL_PARTSTAT_NEEDSACTION));
        icalproperty_add_parameter(prop_attendee,
                                   icalparameter_new_rsvp(ICAL_RSVP_TRUE));

        icalcomponent_add_property(vevent, prop_attendee);
    }

    icalcomponent_add_property(vevent,
                               icalproperty_new_status(ICAL_STATUS_CONFIRMED));

    return vevent;
}
