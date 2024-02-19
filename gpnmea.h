#pragma once
#include "gpbase.h"
#include <memory>
#include <utility>
#include <list>
#include <vector>
#include <chrono>
#include <map>
#include <unordered_map>
#include <functional>
GP_NAMESPACE_BEGIN

constexpr char KNMEAStartC = '$';
constexpr char KNMEACommaC = ',';
constexpr char KNMEACheckSumFieldC = '*';
constexpr char KCR ='\r';
constexpr char KLF = '\n';

constexpr const char* KSTRRMC = "RMC";
constexpr const char* KSTRGGA = "GGA";
constexpr const char* KSTRGSA = "GSA";
constexpr const char* KSTRGSV = "GSV";
constexpr const char* KSTRGLL = "GLL";
constexpr const char* KSTRVTG = "VTG";

constexpr const char* KSTRGP = "GP";
constexpr const char* KSTRGL = "GL";
constexpr const char* KSTRGA = "GA";
constexpr const char* KSTRGB = "GB";
constexpr const char* KSTRQZ = "QZ";
constexpr const char* KSTRGI = "GI";
constexpr const char* KSTRBD = "BD";
constexpr const char* KSTRGN = "GN";
constexpr const char* KSTRGQ = "GQ";

enum class NMEASentenceID : char {
  NotSupport = GP_ERROR,
  RMC,
  GGA,
  GSA,
  GSV,
  GLL,
  VTG
};
constexpr NMEASentenceID KUniqueID[] = {
  NMEASentenceID::RMC,
  NMEASentenceID::GGA,
  NMEASentenceID::GLL,
  NMEASentenceID::VTG
};

enum class NMEATalkerID : char {
  NotSupport = GP_ERROR,
  GP,  // Global Positioning System receiver
  GL,  // GLONASS
  GA,  // Galileo
  GB,  // BeiDou (China)
  QZ,  // QZSS
  GI,  // NavIC, IRNSS (India)
  BD,  // BeiDou (China)
  GN,  // Combination of multiple satellite systems (NMEA 1083)
  GQ,  // QZSS regional GPS augmentation system (Japan)
};
enum class NMEASystemID : char {
  NotSupport = GP_ERROR,
  GPS=1,
  GLONASS,
  Galileo,
  BeiDou,
  QZSS,
  NAVIC,
  ALL
};
enum class NMEAModeInd {
  NotSupport = GP_ERROR,
  Auto = 'A',       // Autonomous (non-differential)
  Diff = 'D',       // Differential mode
  DR = 'E',         // Estimated (dead reckoning) Mode
  Float = 'F',      // Float RTK.
  Manual = 'M',     // Manual Input Mode
  NotFix = 'N',     //  No fix.
  Precise = 'P',    // Precise.
  RTK = 'R',        // Real Time Kinematic.
  Simulator = 'S',  // Simulator Mode
};
enum class NMEANAVStatus {
  NotSupport = GP_ERROR,
  Safe = 'S',     // Safe
  Caution = 'C',  // Caution
  Unsafe = 'U',   // Unsafe
  InValid = 'V'   // Navigational status not valid, equipment is not providing
};

enum class NMEADIR {
  NotSupport = GP_ERROR,
  N = 'N',
  S = 'S',
  E = 'E',
  W = 'W'
};
enum class NMEAGPSQuality {
  NotSupport = GP_ERROR,
  Unavailable = 0,  ///< fix not available,
  SPS = 1,          ///< GPS fix,
  DGPS = 2,         ///< Differential GPS fix(values above 2 are 2.3 features)
  PPS = 3,          ///< GPS PPS Mode, fix valid
  RTK = 4,          ///< Real Time Kinematic
  Float = 5,        ///< Float RTK
  DR = 6,           ///< estimated(dead reckoning)
  Manual = 7,       ///< Manual input mode
  Simulation = 8,   ///< Simulation mode
};
enum class NMEAMode {
  NotSupport = GP_ERROR,
  Manual = 'M',  ///< Manual, forced to operate in2D or 2D mode
  Auto = 'A',    ///< Automatic, allowed to automatically switch 2D/3D
};
enum class NMEAFixMode {
  NotSupport = GP_ERROR,
  Unavailable = 1,  ///< Fix not available
  F2D = 2,          ///< 2D
  F3D = 3,          ///< 3D
};

struct NMEABase {
  enum Error {
      NoError,
      CheckSumError, 
      IncompleteError,
      CRLFError,
      OverLengthError,
      NotAlphabet
  };
  virtual ~NMEABase(){};
  virtual bool Parser(const NMEABase& nmeabase) {
    sentence_id = nmeabase.sentence_id;
    talker_id = nmeabase.talker_id;
    sentence_buffer_ = nmeabase.sentence_buffer_;
    errorcode_ = nmeabase.errorcode_;
    errMsg = nmeabase.errMsg;
    return false;
  }
  NMEASentenceID sentence_id = NMEASentenceID::NotSupport;
  NMEATalkerID talker_id = NMEATalkerID::NotSupport;
  std::string sentence_buffer_ = std::string();
  Error errorcode_ = Error::NoError;
  std::string errMsg = std::string();
};

//input raw data  
class NMEAParser : public GPBase<char> {
 public:
  enum ParserStatus {
    GetStart,
    GetAddressField,
    GetDataBlock,
    GetCheckSum1,
    GetCheckSum2,
    GetCR,
    GetLF
  };
  NMEAParser(int max_Length=82, int address_field_length=5);
  ~NMEAParser();
  InputStatus Input(const char&) override;
  void Reset() override;
  NMEABase GetNMEABase();
 private:
  NMEABase nmea_base_ = NMEABase();
  ParserStatus parser_status_ = ParserStatus::GetStart;
  int max_Length_ = 82;
  int address_field_length_ = 5;
  std::string hex_checksum = std::string();
  //
  void SetError(const char& byte_, NMEABase::Error errorcode = NMEABase::Error::NoError,
                const std::string& msg = std::string());
};

//nmea value  valid =false if empty string
template <typename _Value_TYPE>
struct NMEAValue : public Value<_Value_TYPE> {
  std::string value_str = std::string();
  bool Parser(const std::string&) override { return false; }
};
template <>
struct NMEAValue<double> : public Value<double> {
  std::string value_str = std::string();
  bool Parser(const std::string& item) override {
    if (item.empty() || !std::isdigit(item[0])) {
      valid = false;
      value_str = item;
      return false;
    } else {
      valid = true;
      value = std::stod(item);
      value_str = item;
      return true;
    }
  }
};
template <>
struct NMEAValue<int> : public Value<int> {
  std::string value_str = std::string();
  bool Parser(const std::string& item) override {
    if (item.empty() || !std::isdigit(item[0])) {
      valid = false;
      value_str = item;
      return false;
    } else {
      valid = true;
      value = std::stol(item);
      value_str = item;
      return true;
    }
  }
};
template <>
struct NMEAValue<char> : public Value<char> {
  std::string value_str = std::string();
  bool Parser(const std::string& item) override {
    if (item.empty()) {
      valid = false;
      value_str = item;
      return false;
    } else {
      valid = true;
      value = item[0];
      value_str = item;
      return true;
    }
  }
};
struct NMEARMC : public NMEABase {
  bool Parser(const NMEABase&) override;
  NMEAValue<double> utc = NMEAValue<double>();
  bool status =false;
  NMEAValue<double> lat = NMEAValue<double>();
  NMEADIR lat_dir = NMEADIR::NotSupport;
  NMEAValue<double> lon = NMEAValue<double>();
  NMEADIR lon_dir = NMEADIR::NotSupport;
  NMEAValue<double> sog = NMEAValue<double>();
  NMEAValue<double> cog = NMEAValue<double>();
  NMEAValue<int> date = NMEAValue<int>();
  NMEAValue<double> mag_var = NMEAValue<double>();
  NMEADIR mag_var_dir = NMEADIR::NotSupport;
  NMEAModeInd mode_ind = NMEAModeInd::NotSupport;
  NMEANAVStatus nav_status = NMEANAVStatus::NotSupport;
};
struct NMEAGGA : public NMEABase {
  bool Parser(const NMEABase&) override;
  NMEAValue<double> utc = NMEAValue<double>();
  NMEAValue<double> lat = NMEAValue<double>();
  NMEADIR lat_dir = NMEADIR::NotSupport;
  NMEAValue<double> lon = NMEAValue<double>();
  NMEADIR lon_dir = NMEADIR::NotSupport;
  NMEAGPSQuality quality = NMEAGPSQuality::NotSupport;
  NMEAValue<int> num_sat_used = NMEAValue<int>();
  NMEAValue<double> hdop = NMEAValue<double>();
  NMEAValue<double> alt = NMEAValue<double>();
  NMEAValue<char> alt_unit = NMEAValue<char>();
  NMEAValue<double> sep = NMEAValue<double>();
  NMEAValue<char> sep_unit = NMEAValue<char>();
  NMEAValue<int> diff_age = NMEAValue<int>();
  NMEAValue<int> diff_station = NMEAValue<int>();
};
struct NMEAGSA : public NMEABase {
  bool Parser(const NMEABase&) override;
  NMEAMode mode = NMEAMode::NotSupport;
  NMEAFixMode fix_mode = NMEAFixMode::NotSupport;
  std::list<int> sat_id = std::list<int>();
  NMEAValue<double> pdop = NMEAValue<double>();
  NMEAValue<double> hdop = NMEAValue<double>();
  NMEAValue<double> vdop = NMEAValue<double>();
  NMEASystemID system_id = NMEASystemID::NotSupport;
};
struct NMEAGSVSatInfo {
  NMEAValue<int> sat_id;
  NMEAValue<int> sat_elev;
  NMEAValue<int> sat_az;
  NMEAValue<int> sat_snr;
};

struct NMEAGSV : public NMEABase {
  bool Parser(const NMEABase&) override;
  NMEAValue<int> total_num_stc = NMEAValue<int>();
  NMEAValue<int> stc_num = NMEAValue<int>();
  NMEAValue<int> total_num_sta = NMEAValue<int>();
  std::list<NMEAGSVSatInfo> sat_info = std::list<NMEAGSVSatInfo>();
  NMEAValue<char> sig_id = NMEAValue<char>();
};
struct NMEAVTG : public NMEABase {
  bool Parser(const NMEABase&) override;
  NMEAValue<double> codt;  //Course over ground  degrees True
  NMEAValue <char>  codt_d;
  NMEAValue<double> cogm;  //Course over ground degrees Magnetic
  NMEAValue <char>  cogm_d;
  NMEAValue<double> sogn;  //Speed over ground, km/hr
  NMEAValue <char > sogn_unit;
  NMEAValue<double> sogk;  //Speed over ground, km/hr
  NMEAValue <char>  sogk_unit;
  NMEAModeInd       mode_ind = NMEAModeInd::NotSupport;
};
struct NMEAGLL : public NMEABase {
  bool Parser(const NMEABase&) override;
  NMEAValue<double> lat;
  NMEADIR lat_dir;
  NMEAValue<double> lon;
  NMEADIR lon_dir;
  NMEAValue<double> utc;
  bool status;
  NMEAModeInd mode_ind = NMEAModeInd::NotSupport;
};

struct SolutionSatInfo {
  int sat_elev = 0.0;
  int sat_az = 0.0;
  int sat_snr=0.0;
};
struct SolutionSatSignalInfo {
  bool used = false;
  std::map<char /*sinal id*/, SolutionSatInfo> SatInfos =
      std::map<char, SolutionSatInfo>();
};
struct NMEAClock {
  time_t time=0;/* time (s) expressed by standard time_t */
  double sec =0.0;/* fraction of second under 1 s */
};
using sys_satinfo=std::unordered_map<NMEASystemID,std::unordered_map<int/*sat id*/,SolutionSatSignalInfo>>;
struct NMEASolution {
  bool status = false;
  NMEAClock utc_time = NMEAClock();
  double lat = 0.0, lon = 0.0, alt = 0.0, sep = 0.0;
  double sog = 0.0, cog = 0.0, mag_var = 0.0;
  double pdop = 0.0, hdop = 0.0, vdop = 0.0;
  NMEAGPSQuality quality = NMEAGPSQuality::NotSupport;
  int diff_age = 0, diff_station = 0;
  sys_satinfo sys_info = sys_satinfo();

  NMEASentenceID current_sentence = NMEASentenceID::NotSupport;
  std::unordered_map<NMEASentenceID, bool> current_frames =
      std::unordered_map<NMEASentenceID, bool>();
  std::map<NMEASentenceID, std::size_t> sentence_cout;
  /*call back*/
  std::function<void(NMEASolution)> NMEAFrameUpdate;
};
void Update_Solution(NMEABase&, NMEASolution&);
  GP_NAMESPACE_END