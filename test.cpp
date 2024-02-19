#include <iostream>
#include "gpnmea.h"
int main() {
  std::string nmea =
      "$GNRMC,085143.000,A,3149.332419,N,11706.946516,E,0.012,312.78,060423,,,"
      "R,V*24\r\n "
      "$GNGGA,085145.000,3149.332421,N,11706.946514,E,4,44,1.03,53.2,M,-0.3,M,"
      "1.0,1591*71\r\n";
    gp::NMEAParser nmea_parser;
  for (auto &byte_ : nmea) {
      std::cout << byte_ ;
      auto status = nmea_parser.Input(byte_);
      if (status == gp::InputStatus::Geted) {
        gp::NMEABase nmeabase=     nmea_parser.GetNMEABase();
        switch (nmeabase.sentence_id) {
         case gp::NMEASentenceID::RMC: {
            gp::NMEARMC rmc;
            rmc.Parser(nmeabase);
            break;
         } 
         case gp::NMEASentenceID::GGA: {
           gp::NMEAGGA gga;
           gga.Parser(nmeabase);
           break;
         } 
          default:
            break;
        }
        std::cout << "Get NMEA:"<< nmeabase.sentence_buffer_ << std::endl;
      }
      if (status == gp::InputStatus::ErrorOccur) {
        gp::NMEABase nmeabase = nmea_parser.GetNMEABase();
        std::cout << "ErrorOccur:" << nmeabase.errorcode_ << nmeabase.errMsg
                  << std::endl;
      }
  }
}