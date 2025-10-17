#include "json_to_ical.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <cstring>

// JSON to CalendarEvent conversion
void from_json(const json &j, CalendarEvent &event)
{
    j.at("summary").get_to(event.summary);

    if (j.contains("description"))
    {
        j.at("description").get_to(event.description);
    }

    if (j.contains("location"))
    {
        j.at("location").get_to(event.location);
    }

    j.at("start_datetime").get_to(event.start_datetime);
    j.at("end_datetime").get_to(event.end_datetime);

    if (j.contains("timezone"))
    {
        j.at("timezone").get_to(event.timezone);
    }
    else
    {
        event.timezone = "UTC";
    }

    if (j.contains("uid"))
    {
        j.at("uid").get_to(event.uid);
    }

    if (j.contains("organizer_name"))
    {
        j.at("organizer_name").get_to(event.organizer_name);
    }

    if (j.contains("organizer_email"))
    {
        j.at("organizer_email").get_to(event.organizer_email);
    }

    if (j.contains("attendees"))
    {
        j.at("attendees").get_to(event.attendees);
    }

    if (j.contains("all_day_event"))
    {
        j.at("all_day_event").get_to(event.all_day_event);
    }
}

ICalGenerator::ICalGenerator()
    : prodid_("-//JSON to iCal Converter//EN")
{
    // Create VCALENDAR component
    vcalendar_ = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    // Add VERSION property
    icalcomponent_add_property(vcalendar_,
                               icalproperty_new_version("2.0"));

    // Add PRODID property
    icalcomponent_add_property(vcalendar_,
                               icalproperty_new_prodid(prodid_.c_str()));

    // Add CALSCALE property
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
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        json j;
        file >> j;

        // Support both single event and array of events
        if (j.is_array())
        {
            for (const auto &event_json : j)
            {
                CalendarEvent event;
                from_json(event_json, event);

                // Generate UID if not provided
                if (event.uid.empty())
                {
                    event.uid = generateUID();
                }

                addEvent(event);
            }
        }
        else
        {
            CalendarEvent event;
            from_json(j, event);

            if (event.uid.empty())
            {
                event.uid = generateUID();
            }

            addEvent(event);
        }

        return true;
    }
    catch (const json::exception &e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
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
    // Remove separators if present
    std::string clean_dt = datetime;
    clean_dt.erase(std::remove(clean_dt.begin(), clean_dt.end(), '-'), clean_dt.end());
    clean_dt.erase(std::remove(clean_dt.begin(), clean_dt.end(), ':'), clean_dt.end());

    icaltimetype tt = icaltime_null_time();

    if (isAllDay || clean_dt.length() == 8)
    {
        // Parse date only (YYYYMMDD)
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
        // Parse datetime (YYYYMMDDTHHMMSS)
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
    // Create VEVENT component
    icalcomponent *vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);

    // Add UID
    icalcomponent_add_property(vevent,
                               icalproperty_new_uid(event.uid.c_str()));

    // Add SUMMARY
    icalcomponent_add_property(vevent,
                               icalproperty_new_summary(event.summary.c_str()));

    // Add DESCRIPTION if present
    if (!event.description.empty())
    {
        icalcomponent_add_property(vevent,
                                   icalproperty_new_description(event.description.c_str()));
    }

    // Add LOCATION if present
    if (!event.location.empty())
    {
        icalcomponent_add_property(vevent,
                                   icalproperty_new_location(event.location.c_str()));
    }

    // Add DTSTART
    icaltimetype dtstart = parseDateTime(event.start_datetime, event.all_day_event);
    icalproperty *prop_start = icalproperty_new_dtstart(dtstart);

    if (!event.all_day_event && !event.timezone.empty() && event.timezone != "UTC")
    {
        // Add timezone parameter for non-all-day events
        icalparameter *tzid_param = icalparameter_new_tzid(event.timezone.c_str());
        icalproperty_add_parameter(prop_start, tzid_param);
    }
    icalcomponent_add_property(vevent, prop_start);

    // Add DTEND
    icaltimetype dtend = parseDateTime(event.end_datetime, event.all_day_event);
    icalproperty *prop_end = icalproperty_new_dtend(dtend);

    if (!event.all_day_event && !event.timezone.empty() && event.timezone != "UTC")
    {
        // Add timezone parameter for non-all-day events
        icalparameter *tzid_param = icalparameter_new_tzid(event.timezone.c_str());
        icalproperty_add_parameter(prop_end, tzid_param);
    }
    icalcomponent_add_property(vevent, prop_end);

    // Add DTSTAMP (current time)
    icaltimetype dtstamp = icaltime_current_time_with_zone(icaltimezone_get_utc_timezone());
    icalcomponent_add_property(vevent,
                               icalproperty_new_dtstamp(dtstamp));

    // Add ORGANIZER if present
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

    // Add ATTENDEEs
    for (const auto &attendee_email : event.attendees)
    {
        std::string mailto = "mailto:" + attendee_email;
        icalproperty *prop_attendee = icalproperty_new_attendee(mailto.c_str());

        // Add common attendee parameters
        icalproperty_add_parameter(prop_attendee,
                                   icalparameter_new_role(ICAL_ROLE_REQPARTICIPANT));
        icalproperty_add_parameter(prop_attendee,
                                   icalparameter_new_partstat(ICAL_PARTSTAT_NEEDSACTION));
        icalproperty_add_parameter(prop_attendee,
                                   icalparameter_new_rsvp(ICAL_RSVP_TRUE));

        icalcomponent_add_property(vevent, prop_attendee);
    }

    // Add STATUS
    icalcomponent_add_property(vevent,
                               icalproperty_new_status(ICAL_STATUS_CONFIRMED));

    return vevent;
}
