// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-29
//
// TODO: Provide url_is_special() as flag
//       Use u8string
//       Test urls with % encoded code points
//       Initialize base url with about:blank
//       Test complete authority (userinfo + host + port) using truth table

#include "upnplib/url.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
//#include <fstream>

//
namespace upnplib {

static bool url_is_special(std::string_view a_str) {
    return a_str == "ftp" || a_str == "file" || a_str == "http" ||
           a_str == "https" || a_str == "ws" || a_str == "wss";
}

static bool is_in_userinfo_percent_encode_set(const unsigned char a_chr) {
    return // C0 controls
        (a_chr <= '\x1F') ||
        // C0 control percent-encode set
        a_chr > '\x7E' ||
        // query percent-encode set
        a_chr == ' ' || a_chr == '"' || a_chr == '#' || a_chr == '<' ||
        a_chr == '>' ||
        // path percent-encode set
        a_chr == '?' || a_chr == '`' || a_chr == '{' || a_chr == '}' ||
        // userinfo percent-encode set
        a_chr == '/' || a_chr == ':' || a_chr == ';' || a_chr == '=' ||
        a_chr == '@' || (a_chr >= '[' && a_chr <= '^') || a_chr == '|';
}

static std::string UTF8_percent_encode(const unsigned char a_chr) {
    // Simplified function 'UTF-8 percent-encode' from the URL standard may be
    // adjusted if needed.
    if (is_in_userinfo_percent_encode_set(a_chr)) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::uppercase << std::hex;
        escaped << '%' << std::setw(2) << int((unsigned char)a_chr);
        return escaped.str();
    } else
        return std::string(sizeof(a_chr), a_chr);
}

//
#if false
Url::Url() {
    // Proof to redirect clog to /dev/null, <fstream> is needed
    // save clog stream buffer
    std::streambuf* clog_old = std::clog.rdbuf();
    // Redirect clog
    std::ofstream clog_new("/dev/null");
    std::clog.rdbuf(clog_new.rdbuf());
}

Url::~Url() {
    // restore clog stream buffer
    std::clog.rdbuf(clog_old);
}
#endif

//
Url::operator std::string() const { return m_serialized_url; }

void Url::clear() {
    m_given_url = "";
    this->clear_private();
}

void Url::clear_private() {
    // Clears all properties except m_given_url that may already be set to a new
    // value.
    m_input.reserve(m_given_url.size());
    m_input = "";
    m_buffer.reserve(m_input.size() + 20);
    m_buffer = "";
    m_serialized_url = "";
    m_ser_base_url = "";
    m_scheme = "";
    m_authority = "";
    m_username = "";
    m_password = "";
    m_host = "";
    m_port = "";
    m_port_num = 0;
    m_path = "";
    m_query = "";
    m_fragment = "";
    m_atSignSeen = false;
    m_insideBrackets = false;
    m_passwordTokenSeen = false;
}

std::string Url::scheme() const { return m_scheme; }

std::string Url::authority() const { return m_authority; }

std::string Url::username() const { return m_username; }

std::string Url::host() const { return m_host; }

std::string Url::port() const { return m_port; }

uint16_t Url::port_num() const { return m_port_num; }

std::string Url::path() const { return m_path; }

std::string Url::query() const { return m_query; }

std::string Url::fragment() const { return m_fragment; }

//
void Url::operator=(const std::string& a_given_url) {

    m_given_url = a_given_url;
    this->clear_private();

    // To understand the parser below please refer to the "URL Living Standard"
    // as noted at the top. I use the same terms so you should be able to see
    // the relations better that way.

    // Remove control character and space and copy to input. Because we copy
    // char by char I use a predefined length on input to avoid additional
    // memory allocation for characters.
    this->clean_and_copy_url_to_input();

    m_state = STATE_SCHEME_START;
    m_pointer = m_input.begin();

    // On the URL standard there is a State Machine used. It parses the inpupt
    // string with a pointer to the string so it should finish at the end of
    // it. The loop of the State Machine finishes regular if state is set to
    // STATE_NO_STATE within the Machine. We guard it to always finish
    // independent from the Machines logic to be on the safe side. Because the
    // m_pointer is decreased sometimes in the State Machine we just add 10 to
    // guard.
    int guard = m_input.size() + 10;

    // Because there are no external events we can use this
    // simple Finite State Machine (fsm):
    for (; guard > 0; m_pointer++, guard--) {
        if (m_state == STATE_NO_STATE)
            break;

        switch (m_state) {
        case STATE_SCHEME_START:
            this->fsm_scheme_start();
            break;
        case STATE_SCHEME:
            this->fsm_scheme();
            break;
        case STATE_NO_SCHEME:
            this->fsm_no_scheme();
            break;
        case STATE_PATH_OR_AUTHORITY:
            this->fsm_path_or_authority();
            break;
        case STATE_SPECIAL_AUTHORITY_SLASHES:
            this->fsm_special_authority_slashes();
            break;
        case STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES:
            this->fsm_special_authority_ignore_slashes();
            break;
        case STATE_AUTHORITY:
            this->fsm_authority();
            break;
        case STATE_HOST:
            this->fsm_host();
            break;
        case STATE_FILE:
            this->fsm_file();
            break;
        case STATE_SPECIAL_RELATIVE_OR_AUTHORITY:
            this->fsm_special_relative_or_authority();
            break;
        case STATE_PATH:
            this->fsm_path();
            break;
        case STATE_OPAQUE_PATH:
            this->fsm_opaque_path();
            break;
        default:
            guard = 0;
            break;
        }
    }

    if (guard <= 0) {
        throw std::out_of_range(
            std::string((std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                        ", Parsing URL " + __func__ +
                        ". State Machine doesn't finish regular."));
    }
}

//
void Url::clean_and_copy_url_to_input() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'clean_and_copy_url_to_input'.\n";
#endif

    // Copy given URL to input lowercase and remove all control chars and space.
    for (auto it = m_given_url.begin(); it < m_given_url.end(); it++) {
        // control chars are \x00 to \x1F, space = \x20, DEL (backspace) = \x7F
        if (*it > ' ' && *it != '\x7F')
            // control chars are \x00 to \x1F and space = '\x20',
            // but DEL (backspace) = '\x7F' ignored?
            // if (*it > ' ')
            m_input.push_back(std::tolower(*it));
    }
    if (m_input.size() != m_given_url.size())
        std::clog << "Warning: Removed " << m_given_url.size() - m_input.size()
                  << " ASCII control character or spaces. Using \"" << m_input
                  << "\" now." << std::endl;
}

//
void Url::fsm_scheme_start() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'scheme_start_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    // Check if first character is an lower ASCII alpha.
    // We should have already converted all chars to lower.
    if (std::islower((unsigned char)*m_pointer)) { // needs type cast here

        // Exception: if the operation would result in size() > max_size(),
        // throws std::length_error.
        m_buffer.push_back(std::tolower(*m_pointer));

        m_state = STATE_SCHEME;

    } else {

        m_state = STATE_NO_SCHEME;
        m_pointer--;
    }
}

//
void Url::fsm_scheme() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'scheme state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    const unsigned char c = *m_pointer;

    // Check if character is an ASCII lower alphanumeric or U+002B (+), U+002D
    // (-), or U+002E (.).
    if (islower(c) || // type cast is needed here
        isdigit(c) || c == '+' || c == '-' || c == '.') //
    {
        // Exception: if the operation would result in size() > max_size(),
        // throws std::length_error.
        m_buffer.push_back(c);

    } else if (c == ':') {

        m_scheme = m_buffer;
        m_buffer = "";

        if (m_scheme == "file") {
            if (m_pointer + 2 >= m_input.end() || *(m_pointer + 1) != '/' ||
                *(m_pointer + 2) != '/')
                std::clog << "Warning: 'file' scheme misses \"//\", ignoring."
                          << std::endl;
            m_state = STATE_FILE;

        } else if (url_is_special(m_scheme) && m_ser_base_url != "") {
            m_state = STATE_SPECIAL_RELATIVE_OR_AUTHORITY;

        } else if (url_is_special(m_scheme)) {
            m_state = STATE_SPECIAL_AUTHORITY_SLASHES;

        } else if (m_pointer + 1 < m_input.end() && *(m_pointer + 1) == '/') {
            m_state = STATE_PATH_OR_AUTHORITY;
            m_pointer++;

        } else {
            m_path = "";
            m_state = STATE_OPAQUE_PATH;
        }

    } else {

        m_buffer = "";
        m_state = STATE_NO_SCHEME;
    }
}

//
void Url::fsm_no_scheme() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'no_scheme_state' with input \"" << m_input
              << "\"\n";
#endif
    std::clog << "Error: no valid scheme found." << std::endl;
    throw std::invalid_argument("Invalid URL: '" + m_input + "'");

    m_state = STATE_NO_STATE;
}

//
void Url::fsm_special_relative_or_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'special_relative_or_authority_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    m_state = STATE_NO_STATE;
}

//
void Url::fsm_path_or_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'path_or_authority_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    if (*m_pointer == '/') {
        m_state = STATE_AUTHORITY;
    } else {
        m_state = STATE_PATH;
        m_pointer--;
    }
}

//
void Url::fsm_special_authority_slashes() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'special_authority_slashes_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    if (m_pointer + 1 < m_input.end() && *m_pointer == '/' &&
        *(m_pointer + 1) == '/') {
        m_state = STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
        m_pointer++;
    } else {
        std::clog << "Warning: no \"//\" before authority: ignoring. Found \""
                  << std::string(m_pointer, m_input.end()) << "\"" << std::endl;
        m_state = STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
        m_pointer--;
    }
}

//
void Url::fsm_special_authority_ignore_slashes() {
#ifdef DEBUG_URL
    std::clog
        << "DEBUG: Being on 'special_authority_ignore_slashes_state' with \""
        << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    if (*m_pointer != '/' && *m_pointer != '\\') {
        m_state = STATE_AUTHORITY;
        m_pointer--;
    } else {
        std::clog << "Warning: '/' or '\\' not expected on authority: "
                     "ignoring. Found \""
                  << std::string(m_pointer, m_input.end()) << "\"" << std::endl;
    }
}

//
void Url::fsm_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'authority_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    const unsigned char c = *m_pointer;

    if (c == '@') {

        std::clog << "Status: '@' found for userinfo." << std::endl;
        if (m_atSignSeen)
            m_buffer.append("%40");
        else
            m_atSignSeen = true;

        for (auto& cp : m_buffer) {
            if (cp == ':' && !m_passwordTokenSeen) {
                m_passwordTokenSeen = true;
                continue;
            }
            std::string encodedCodePoints = UTF8_percent_encode(cp);
            if (m_passwordTokenSeen)
                m_password += encodedCodePoints;
            else
                m_username += encodedCodePoints;
        }
        m_buffer = "";

    } else if (m_pointer >= m_input.end() || c == '/' || c == '?' || c == '#' ||
               (url_is_special(m_scheme) && c == '\\')) {

        if (m_atSignSeen && m_buffer == "") {
            std::clog << "Error: no valid authority." << std::endl;
            throw std::invalid_argument("Invalid authority: '" + m_input + "'");
        } else {
            m_pointer = m_pointer - m_buffer.length() - 1;
            m_buffer = "";
            m_state = STATE_HOST;
        }

    } else {
        m_buffer.push_back(c);
    }
}

//
void Url::fsm_host() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'host_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\", "
              << "username = \"" << m_username << "\", password = \""
              << m_password << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

//
void Url::fsm_file() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'file_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

//
void Url::fsm_path() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'path_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

//
void Url::fsm_opaque_path() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'opaque_path_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

} // namespace upnplib
