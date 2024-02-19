#include "gpnmea.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <format>
GP_NAMESPACE_BEGIN
/*#############help func###################*/
static NMEASentenceID GetSentenceID(std::string sentenceid_str) {
  if (!sentenceid_str.compare(KSTRRMC)) {
    return NMEASentenceID::RMC;
  }
  if (!sentenceid_str.compare(KSTRGGA)) {
    return NMEASentenceID::GGA;
  }
  if (!sentenceid_str.compare(KSTRGSA)) {
    return NMEASentenceID::GSA;
  }
  if (!sentenceid_str.compare(KSTRGSV)) {
    return NMEASentenceID::GSV;
  }
  if (!sentenceid_str.compare(KSTRGLL)) {
    return NMEASentenceID::GLL;
  }
  if (!sentenceid_str.compare(KSTRVTG)) {
    return NMEASentenceID::VTG;
  }
  return NMEASentenceID::NotSupport;
}
static NMEATalkerID GetTalkerID(std::string talkerid_str) {
  if (!talkerid_str.compare(KSTRGP)) {
    return NMEATalkerID::GP;
  }
  if (!talkerid_str.compare(KSTRGL)) {
    return NMEATalkerID::GL;
  }
  if (!talkerid_str.compare(KSTRGA)) {
    return NMEATalkerID::GA;
  }
  if (!talkerid_str.compare(KSTRGB)) {
    return NMEATalkerID::GB;
  }
  if (!talkerid_str.compare(KSTRQZ)) {
    return NMEATalkerID::QZ;
  }
  if (!talkerid_str.compare(KSTRGI)) {
    return NMEATalkerID::GI;
  }
  if (!talkerid_str.compare(KSTRBD)) {
    return NMEATalkerID::BD;
  }
  if (!talkerid_str.compare(KSTRGN)) {
    return NMEATalkerID::GN;
  }
  if (!talkerid_str.compare(KSTRGQ)) {
    return NMEATalkerID::GQ;
  }
  return NMEATalkerID::NotSupport;
}
static std::list<std::string> GetItems(std::string sentence) {
  std::list<std::string> items = std::list<std::string>();
  if (sentence.empty()) {
    return items;
  }
  int off = 0, pos = 0;
  while (true) {
    pos = sentence.find(',', off);
    if (pos == std::string::npos) {
      break;
    }
    std::string item(sentence.substr(off, pos - off));
    items.push_back(std::move(item));
    off = pos + 1;
  }
  pos = sentence.find('*', off);
  if (pos == std::string::npos) {
    return std::list<std::string>();
  }
  std::string item(sentence.substr(off, pos - off));
  items.push_back(std::move(item));
  items.pop_front();
  return items;
}
static NMEADIR GetDirection(const char& item) {
  NMEADIR status = NMEADIR::NotSupport;
  if (item == (char)NMEADIR::E) {
    status = NMEADIR::E;
    return status;
  }
  if (item == (char)NMEADIR::N) {
    status = NMEADIR::N;
    return status;
  }
  if (item == (char)NMEADIR::S) {
    status = NMEADIR::S;
    return status;
  }
  if (item == (char)NMEADIR::W) {
    status = NMEADIR::W;
    return status;
  }
  return status;
}
static NMEAModeInd GetModeIndicator(const char& item) {
  NMEAModeInd status = NMEAModeInd::NotSupport;

  if (item == (char)NMEAModeInd::Auto) {
    status = NMEAModeInd::Auto;
    return status;
  }
  if (item == (char)NMEAModeInd::Diff) {
    status = NMEAModeInd::Diff;
    return status;
  }
  if (item == (char)NMEAModeInd::DR) {
    status = NMEAModeInd::DR;
    return status;
  }
  if (item == (char)NMEAModeInd::Float) {
    status = NMEAModeInd::Float;
    return status;
  }
  if (item == (char)NMEAModeInd::Manual) {
    status = NMEAModeInd::Manual;
    return status;
  }
  if (item == (char)NMEAModeInd::NotFix) {
    status = NMEAModeInd::NotFix;
    return status;
  }
  if (item == (char)NMEAModeInd::Precise) {
    status = NMEAModeInd::Precise;
    return status;
  }
  if (item == (char)NMEAModeInd::RTK) {
    status = NMEAModeInd::RTK;
    return status;
  }
  if (item == (char)NMEAModeInd::Simulator) {
    status = NMEAModeInd::Simulator;
    return status;
  }
  return status;
}
static NMEANAVStatus GetNavStatus(const char& item) {
  NMEANAVStatus status = NMEANAVStatus::NotSupport;

  if (item == (char)NMEANAVStatus::Caution) {
    status = NMEANAVStatus::Caution;
    return status;
  }
  if (item == (char)NMEANAVStatus::Safe) {
    status = NMEANAVStatus::Safe;
    return status;
  }
  if (item == (char)NMEANAVStatus::Unsafe) {
    status = NMEANAVStatus::Unsafe;
    return status;
  }
  if (item == (char)NMEANAVStatus::InValid) {
    status = NMEANAVStatus::InValid;
    return status;
  }
  return status;
}
static NMEAMode GetMode(const char& item) {
  NMEAMode status = NMEAMode::NotSupport;
  if (item == (char)NMEAMode::Auto) {
    status = NMEAMode::Auto;
    return status;
  }
  if (item == (char)NMEAMode::Manual) {
    status = NMEAMode::Manual;
    return status;
  }
  return status;
}
static NMEAGPSQuality GetNMEAGPSQuality(const char& item) {
  if (item < (char)NMEAGPSQuality::SPS ||
      item > (char)NMEAGPSQuality::Simulation) {
    return NMEAGPSQuality::NotSupport;
  }
  return (NMEAGPSQuality)item;
}
static NMEAFixMode GetNMEAFixMode(const char& item) {
  if (item < (char)NMEAFixMode::Unavailable || item > (char)NMEAFixMode::F3D) {
    return NMEAFixMode::NotSupport;
  }
  return (NMEAFixMode)item;
}
static NMEASystemID GetNMEASystemID(const char& item) {
  if (item < (char)NMEASystemID::GPS || item > (char)NMEASystemID::NAVIC) {
    return NMEASystemID::NotSupport;
  }
  return (NMEASystemID)item;
}
static NMEASystemID TalkerID2SystemID(NMEATalkerID id) {
  switch (id) {
    case gp::NMEATalkerID::NotSupport:
      break;
    case gp::NMEATalkerID::GP:
      return NMEASystemID::GPS;
    case gp::NMEATalkerID::GL:
      return NMEASystemID::GLONASS;
    case gp::NMEATalkerID::GA:
      return NMEASystemID::Galileo;
    case gp::NMEATalkerID::BD:
    case gp::NMEATalkerID::GB:
      return NMEASystemID::BeiDou;
    case gp::NMEATalkerID::GQ:
    case gp::NMEATalkerID::QZ:
      return NMEASystemID::QZSS;
    case gp::NMEATalkerID::GI:
      return NMEASystemID::NAVIC;
    case gp::NMEATalkerID::GN:
      return NMEASystemID::ALL;
  }
}

/* convert calendar day/time to time -------------------------------------------
* convert calendar day/time to gtime_t struct
* args   : double *ep       I   day/time {year,month,day,hour,min,sec}
* return : gtime_t struct
* notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
*/
static NMEAClock Epoch2Time(const double* ep){
  const int doy[] = {1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
  NMEAClock time = {0};
  int days, sec, year = (int)ep[0], mon = (int)ep[1], day = (int)ep[2];

  if (year < 1970 || 2099 < year || mon < 1 || 12 < mon)
    return time;

  /* leap year if year%4==0 in 1901-2099 */
  days = (year - 1970) * 365 + (year - 1969) / 4 + doy[mon - 1] + day - 2 +
         (year % 4 == 0 && mon >= 3 ? 1 : 0);
  sec = (int)floor(ep[5]);
  time.time = (time_t)days * 86400 + (int)ep[3] * 3600 + (int)ep[4] * 60 + sec;
  time.sec = ep[5] - sec;
  return time;
}
/* time to calendar day/time ---------------------------------------------------
* convert gtime_t struct to calendar day/time
* args   : gtime_t t        I   gtime_t struct
*          double *ep       O   day/time {year,month,day,hour,min,sec}
* return : none
* notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
*-----------------------------------------------------------------------------*/
extern void Time2Epoch(NMEAClock t, double* ep) {
  const int mday[] = {/* # of days in a month */
                      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
                      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
                      31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
                      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int days, sec, mon, day;

  /* leap year if year%4==0 in 1901-2099 */
  days = (int)(t.time / 86400);
  sec = (int)(t.time - (time_t)days * 86400);
  for (day = days % 1461, mon = 0; mon < 48; mon++) {
    if (day >= mday[mon])
      day -= mday[mon];
    else
      break;
  }
  ep[0] = 1970 + days / 1461 * 4 + mon / 12;
  ep[1] = mon % 12 + 1;
  ep[2] = day + 1;
  ep[3] = sec / 3600;
  ep[4] = sec % 3600 / 60;
  ep[5] = sec % 60 + t.sec;
}
/* string to time --------------------------------------------------------------
* convert substring in string to gtime_t struct
* args   : char   *s        I   string ("... yyyy mm dd hh mm ss ...")
*          int    i,n       I   substring position and width
*          gtime_t *t       O   gtime_t struct
* return : status (0:ok,0>:error)
*-----------------------------------------------------------------------------*/
extern int str2time(const char* s, int i, int n, NMEAClock* t) {
  double ep[6];
  char str[256], *p = str;

  if (i < 0 || (int)strlen(s) < i || (int)sizeof(str) - 1 < i)
    return -1;
  for (s += i; *s && --n >= 0;)
    *p++ = *s++;
  *p = '\0';
  if (sscanf(str, "%lf %lf %lf %lf %lf %lf", ep, ep + 1, ep + 2, ep + 3, ep + 4,
             ep + 5) < 6)
    return -1;
  if (ep[0] < 100.0)
    ep[0] += ep[0] < 80.0 ? 2000.0 : 1900.0;
  *t = Epoch2Time(ep);
  return 0;
}
/*#############parser###############*/
NMEAParser::NMEAParser(int max_Length, int address_field_length)
    : max_Length_{max_Length}, address_field_length_{address_field_length} {}
NMEAParser::~NMEAParser() {}
void NMEAParser::Reset() {
  status_buffer.clear();
  parser_status_ = ParserStatus::GetStart;
}

NMEABase NMEAParser::GetNMEABase() {
  return nmea_base_;
}
void NMEAParser::SetError(const char& byte_, NMEABase::Error errorcode,
                          const std::string& msg) {
  if (errorcode != NMEABase::Error::NoError) {
    nmea_base_.errorcode_ = errorcode;
    nmea_base_.errMsg = msg;
  }
  status_buffer.push_back(byte_);
  nmea_base_.sentence_buffer_ = std::move(status_buffer);
  status_buffer.clear();
}
InputStatus NMEAParser::Input(const char& byte_) {
  switch (parser_status_) {
    case NMEAParser::GetStart: {
      if (byte_ == KNMEAStartC) {
        status_buffer.clear();
        status_buffer.push_back(byte_);
        parser_status_ = NMEAParser::GetAddressField;
      }
      return InputStatus::Geting;
    }
    case NMEAParser::GetAddressField: {
      if (byte_ == KNMEAStartC) {
        status_buffer.clear();
        status_buffer.push_back(byte_);
        parser_status_ = NMEAParser::GetAddressField;
        return InputStatus::Geting;
      }
      if (byte_ == KNMEACheckSumFieldC ) {
          SetError(byte_, NMEABase::IncompleteError,
                   "IncompleteError in GetAddressField!");
          parser_status_ = NMEAParser::GetStart;
          return InputStatus::ErrorOccur;
      }
      if (byte_ == KCR || byte_ == KLF) {
        SetError(byte_, NMEABase::IncompleteError,
                 "IncompleteError in GetAddressField!");
        parser_status_ = NMEAParser::GetStart;
        return InputStatus::ErrorOccur;
      }
      if (byte_ == KNMEACommaC) {
        status_buffer.push_back(byte_);
        nmea_base_.sentence_id = GetSentenceID(status_buffer.substr(3,3));
        nmea_base_.talker_id = GetTalkerID(status_buffer.substr(1,2));
        parser_status_ = NMEAParser::GetDataBlock;
        return InputStatus::Geting;
      }
      if (status_buffer.size() > (address_field_length_ + 1)) {
        SetError(byte_, NMEABase::OverLengthError,
                 "OverLengthError in GetAddressField!");
        parser_status_ = NMEAParser::GetStart;
        return InputStatus::ErrorOccur;
      }
      status_buffer.push_back(byte_);
      return InputStatus::Geting;
    }
    case NMEAParser::GetDataBlock: {
      if (byte_ == KNMEAStartC) {
        SetError(byte_);
        status_buffer.push_back(byte_);
        parser_status_ = NMEAParser::GetAddressField;
        return InputStatus::Geting;
      }
      if (byte_ == KNMEACheckSumFieldC) {
        status_buffer.push_back(byte_);
        parser_status_ = NMEAParser::GetCheckSum1;
        return InputStatus::Geting;
      }
      if (byte_ == KCR || byte_ == KLF) {
        SetError(byte_, NMEABase::IncompleteError,
                 "IncompleteError in GetDataBlock!");
        parser_status_ = NMEAParser::GetStart;
        return InputStatus::ErrorOccur;
      }
      if (status_buffer.size() > max_Length_) {
        SetError(byte_, NMEABase::OverLengthError,
                 "OverLengthError in GetDataBlock!");
        parser_status_ = NMEAParser::GetStart;
        return InputStatus::ErrorOccur;
      }
      status_buffer.push_back(byte_);
      return InputStatus::Geting;
    }
    case NMEAParser::GetCheckSum1: {
      char calculate_checkSum = 0;
      for (int i = 1; i < status_buffer.size() - 1; i++) {
        calculate_checkSum ^= status_buffer[i];
      }
      std::stringstream ss;
      ss << std::hex << std::setw(2) << std::setfill('0')
         << (int)calculate_checkSum;
      ss >> hex_checksum;
      if (hex_checksum[0] != byte_) {
        SetError(byte_, NMEABase::CheckSumError,
                 "CheckSumError in GetCheckSum1!");
        parser_status_ = NMEAParser::GetStart;
        return InputStatus::ErrorOccur;
      }
      status_buffer.push_back(byte_);
      parser_status_ = NMEAParser::GetCheckSum2;
      return InputStatus::Geting;
    }
    case NMEAParser::GetCheckSum2: {
      if (hex_checksum[1] != byte_) {
        SetError(byte_, NMEABase::CheckSumError,
                 "CheckSumError in GetCheckSum2!");
        parser_status_ = NMEAParser::GetStart;
        return InputStatus::ErrorOccur;
      }
      status_buffer.push_back(byte_);
      parser_status_ = NMEAParser::GetCR;
      return InputStatus::Geting;
    }
    case NMEAParser::GetCR: {
      if (byte_ == KCR) {
        parser_status_ = NMEAParser::GetLF;
        status_buffer.push_back(byte_);
        return InputStatus::Geting;
      }
      if (byte_ == KLF) {
        status_buffer.push_back(byte_);
        parser_status_ = NMEAParser::GetStart;
        nmea_base_.sentence_buffer_ = status_buffer;
        nmea_base_.errorcode_ = NMEABase::NoError;
        return InputStatus::Geted;
      }
      SetError(byte_, NMEABase::IncompleteError, "IncompleteError in GetCR!");
      parser_status_ = NMEAParser::GetStart;
      return InputStatus::ErrorOccur;
    }
    case NMEAParser::GetLF: {
      if (byte_ == KLF) {
        status_buffer.push_back(byte_);
        parser_status_ = NMEAParser::GetStart;
        nmea_base_.sentence_buffer_ = status_buffer;
        nmea_base_.errorcode_ = NMEABase::NoError;
        return InputStatus::Geted;
      } else {
        SetError(byte_, NMEABase::IncompleteError, "IncompleteError in GetLF!");
        parser_status_ = NMEAParser::GetStart;
        return InputStatus::ErrorOccur;
      }
    }
  }
  return InputStatus::ErrorOccur;
}

/*#############RMC###############*/
bool NMEARMC::Parser(const NMEABase& nmeabase) {
  NMEABase::Parser(nmeabase);
  auto items = GetItems(sentence_buffer_);
  if (items.empty()) return false;
  utc.Parser(items.front());
  items.pop_front();

  if (items.empty()) return false;
  status = items.front()[0] == 'A' ? true : false;
  items.pop_front();

  if (items.empty()) return false;
  lat.Parser(items.front());
  items.pop_front();

  if (items.empty()) return false;
  lat_dir = GetDirection(items.front().empty()? ' ': items.front()[0]);
  items.pop_front();

  if (items.empty()) return false;
  lon.Parser(items.front());
  items.pop_front();

  if (items.empty()) return false;
  lon_dir = GetDirection(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty()) return false;
  sog.Parser(items.front());
  items.pop_front();

  if (items.empty()) return false;
  cog.Parser(items.front());
  items.pop_front();

  if (items.empty()) return false;
  date.Parser(items.front());
  items.pop_front();

  if (items.empty()) return false;
  mag_var.Parser(items.front());
  items.pop_front();

  if (items.empty()) return false;
  mag_var_dir=GetDirection(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty()) return false;
  mode_ind = GetModeIndicator(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty()) return false;
  nav_status = GetNavStatus(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();
  return true;
}
/*#############GGA###############*/
bool NMEAGGA::Parser(const NMEABase& nmeabase) {
  NMEABase::Parser(nmeabase);
  auto items = GetItems(sentence_buffer_);
  if (items.empty())
    return false;
  utc.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  lat.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  lat_dir = GetDirection(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty())
    return false;
  lon.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  lon_dir = GetDirection(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty())
    return false;
  quality = GetNMEAGPSQuality(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty())
    return false;
  num_sat_used.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  hdop.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  alt.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  alt_unit.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  sep.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  sep_unit.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  diff_age.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  diff_station.Parser(items.front());
  items.pop_front();
  return true;
}
/*#############GSA###############*/
bool NMEAGSA::Parser(const NMEABase& nmeabase) {
  NMEABase::Parser(nmeabase);
  auto items = GetItems(sentence_buffer_);

  if (items.empty())
    return false;
  mode = GetMode(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty())
    return false;
  fix_mode = GetNMEAFixMode(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  for (int i = 0; i < 12; i++) {
    if (items.empty())
      return false;
    NMEAValue<int> id;
    if (!items.front().empty()) {
      id.Parser(items.front());
      sat_id.push_back(std::move(id.value));
    }
    items.pop_front();
  }
  if (items.empty())
    return false;
  pdop.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  hdop.Parser(items.front());
  items.pop_front();

  if (items.empty())
  return false;
  vdop.Parser(items.front());
  items.pop_front();

  if (items.empty())
  return false;
  system_id = GetNMEASystemID(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();
  return true;
}
/*#############GSV###############*/
bool NMEAGSV::Parser(const NMEABase& nmeabase) {
  NMEABase::Parser(nmeabase);
  auto items = GetItems(sentence_buffer_);

  if (items.empty())
    return false;
  total_num_stc.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  stc_num.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  total_num_sta.Parser(items.front());
  items.pop_front();

  int remaind = total_num_stc.value - stc_num.value;
  int total_dsize = remaind > 0 ? 4 : (total_num_sta.value % 4);

  for (int i = 0; i < total_dsize; i++) {
    NMEAGSVSatInfo satinfo;
    if (items.empty())
      return false;
    satinfo.sat_id.Parser(items.front());
    items.pop_front();
    satinfo.sat_elev.Parser(items.front());
    items.pop_front();
    satinfo.sat_az.Parser(items.front());
    items.pop_front();
    satinfo.sat_snr.Parser(items.front());
    items.pop_front();

    sat_info.push_back(std::move(satinfo));
  }
  if (items.empty()) {
    return false;
  }
  sig_id.Parser(items.front());
  items.pop_front();
  return true;
}
/*#############VTG###############*/
bool NMEAVTG::Parser(const NMEABase& nmeabase) {
  NMEABase::Parser(nmeabase);
  auto items = GetItems(sentence_buffer_);

  if (items.empty())
    return false;
  codt.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  codt_d.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  cogm.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  cogm_d.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  sogn.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  sogn_unit.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  sogk.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  sogk_unit.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  mode_ind=GetModeIndicator(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();
  return true;
}
/*#############GLL###############*/
bool NMEAGLL::Parser(const NMEABase& nmeabase) {
  NMEABase::Parser(nmeabase);
  auto items = GetItems(sentence_buffer_);

  if (items.empty())
    return false;
  lat.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  lat_dir = GetDirection(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty())
    return false;
  lon.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  lon_dir = GetDirection(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();

  if (items.empty())
    return false;
  utc.Parser(items.front());
  items.pop_front();

  if (items.empty())
    return false;
  status = items.front()[0] == 'A' ? true : false;
  items.pop_front();

  if (items.empty())
    return false;
  mode_ind = GetModeIndicator(items.front().empty() ? ' ' : items.front()[0]);
  items.pop_front();
  return true;
}

static bool CheckNewFrame(
    NMEASentenceID sentenceid,
    const std::unordered_map<NMEASentenceID, bool>& current_frames) {
  int size = sizeof(KUniqueID) / sizeof(KUniqueID[0]);
  for (int i = 0; i < size; i++) {
    if (sentenceid == KUniqueID[i] && current_frames.contains(sentenceid)) {
          return true;
      }
  }
  return false;
}
static bool CheckHasSatInfo(
    const std::unordered_map<NMEASentenceID, bool>& current_frames) {
  if (current_frames.contains(NMEASentenceID::GSA) ||
      current_frames.contains(NMEASentenceID::GSV)) {
    return true;
  }
  return false;
}
static void Date2Epoch(double v,double *epoch) {
  epoch[0] = v - (int)(v / 1e2) * 1e2;
  epoch[1] = (int)(v - (int)(v / 1e4) * 1e4) / (int)1e2;
  epoch[2] = int(v / 1e4);
}
static void Time2Epoch(double v, double* epoch) {
  epoch[3] = int(v / 1e4);
  epoch[4] = (int)(v - (int)(v / 1e4) * 1e4) / (int)1e2;
  epoch[5] = v - (int)(v / 1e2) * 1e2;
}
void Update_Solution(NMEABase& nmea, NMEASolution& sol) {
  /*update current sentence id*/
  sol.current_sentence = nmea.sentence_id;
  /*sentence cout*/
  sol.sentence_cout[nmea.sentence_id] =
      sol.sentence_cout.contains(nmea.sentence_id)
          ? sol.sentence_cout[nmea.sentence_id]++
          : 1;
  /*check is new frame*/
  if (CheckNewFrame(nmea.sentence_id, sol.current_frames)) {
    if (CheckHasSatInfo(sol.current_frames)) { /* 10hz only one gsa/gsv*/
      sol.sys_info.clear();
    }
    /*frame get callback*/
    if (sol.NMEAFrameUpdate)
      sol.NMEAFrameUpdate(sol);
    sol.current_frames.clear();
  }
  sol.current_frames[nmea.sentence_id] = true;
  switch (nmea.sentence_id) {
     case NMEASentenceID::RMC: {
       NMEARMC* rmc = static_cast<NMEARMC*>(&nmea);
       double epoch[6]{0.0};
       double date = rmc->date.valid ? rmc->date.value : 0.0;
       double time = rmc->utc.valid ? rmc->utc.value : 0.0;
       Date2Epoch(date, epoch);
       Time2Epoch(time, epoch);
       sol.utc_time= Epoch2Time(epoch);
       sol.lat = rmc->lat.valid ? rmc->lat_dir == NMEADIR::N ? rmc->lat.value
                                                             : -rmc->lat.value
                                : 0.0;
       sol.lon = rmc->lon.valid ? rmc->lon_dir == NMEADIR::N ? rmc->lon.value
                                                             : -rmc->lon.value
                                : 0.0;
       sol.sog = rmc->sog.valid ? rmc->sog.value : 0.0;
       sol.cog = rmc->cog.valid ? rmc->cog.value : 0.0;
       sol.mag_var = rmc->mag_var.valid ? rmc->mag_var.value : 0.0;
       sol.status = rmc->status;
       break;
     }
     case NMEASentenceID::GGA: {
       NMEAGGA* gga = static_cast<NMEAGGA*>(&nmea);
       double epoch[6]{0};
       Time2Epoch(sol.utc_time, epoch);
       double time = gga->utc.valid ? gga->utc.value : 0.0;
       Time2Epoch(time, epoch);
       sol.utc_time = Epoch2Time(epoch);
       sol.lat = gga->lat.valid ? gga->lat_dir == NMEADIR::N ? gga->lat.value
                                                             : -gga->lat.value
                                : 0.0;
       sol.lon = gga->lon.valid ? gga->lon_dir == NMEADIR::N ? gga->lon.value
                                                             : -gga->lon.value
                                : 0.0;
       sol.quality = gga->quality;
       sol.hdop = gga->hdop.valid ? gga->hdop.value : 0.0;
       sol.alt = gga->alt.valid ? gga->alt.value : 0.0;
       sol.sep = gga->sep.valid ? gga->sep.value : 0.0;
       sol.diff_age = gga->diff_age.valid ? gga->diff_age.value : 0.0;
       sol.diff_station =gga->diff_station.valid ? gga->diff_station.value : 0.0;
       break;
     }
     case NMEASentenceID::GLL: {
       NMEAGLL* gll = static_cast<NMEAGLL*>(&nmea);
       double epoch[6]{0};
       Time2Epoch(sol.utc_time, epoch);
       double time = gll->utc.valid ? gll->utc.value : 0.0;
       Time2Epoch(time, epoch);
       sol.utc_time = Epoch2Time(epoch);
       sol.lat = gll->lat.valid ? gll->lat_dir == NMEADIR::N ? gll->lat.value
                                                             : -gll->lat.value
                                : 0.0;
       sol.lon = gll->lon.valid ? gll->lon_dir == NMEADIR::N ? gll->lon.value
                                                             : -gll->lon.value
                                : 0.0;
       sol.status = gll->status;
       break;
     }
     case NMEASentenceID::GSA: {
       NMEAGSA* gsa = static_cast<NMEAGSA*>(&nmea);
       NMEASystemID id = NMEASystemID::NotSupport;
       if (gsa->talker_id != NMEATalkerID::GN) {
         id=TalkerID2SystemID(gsa->talker_id);
       } else {
         id = gsa->system_id;
       }
       std::unordered_map<int, SolutionSatSignalInfo> satinfos =
           sol.sys_info.contains(id)
               ? sol.sys_info[id]
               : std::unordered_map<int, SolutionSatSignalInfo>();
       for (auto& satid : gsa->sat_id) {
         SolutionSatSignalInfo solsatsig = SolutionSatSignalInfo();
         solsatsig.used = true;
         satinfos[satid] = solsatsig;
       }
       sol.sys_info[id] = satinfos;
       sol.pdop = gsa->pdop.valid ? gsa->pdop.value : 0.0;
       sol.hdop = gsa->hdop.valid ? gsa->hdop.value : 0.0;
       sol.vdop = gsa->vdop.valid ? gsa->vdop.value : 0.0;
       break;
     }
     case NMEASentenceID::GSV: {
       NMEAGSV* gsv = static_cast<NMEAGSV*>(&nmea);
       NMEASystemID id = NMEASystemID::NotSupport;
       id = TalkerID2SystemID(gsv->talker_id);
       std::unordered_map<int, SolutionSatSignalInfo> satinfos =
           sol.sys_info.contains(id)
               ? sol.sys_info[id]
               : std::unordered_map<int, SolutionSatSignalInfo>();
       for (auto& satinfo : gsv->sat_info) {
         if (!satinfo.sat_id.valid)
           continue;
         SolutionSatSignalInfo solsignalInfo =
             satinfos.contains(satinfo.sat_id.value)
                 ? satinfos[satinfo.sat_id.value]
                 : SolutionSatSignalInfo();
         char sigid = gsv->sig_id.valid ? gsv->sig_id.value : '1';
         SolutionSatInfo satinfo_ = SolutionSatInfo(
             satinfo.sat_elev.value, satinfo.sat_az.value, satinfo.sat_snr.value);

         solsignalInfo.SatInfos[sigid] = satinfo_;
         satinfos[satinfo.sat_id.value] = solsignalInfo;
       }
       break;
     }
     default:
          break;
    }
}


GP_NAMESPACE_END
