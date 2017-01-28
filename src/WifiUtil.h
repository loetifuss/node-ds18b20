class WifiUtil {
  const char* ssid;
  const char* password;

  public:
  WifiUtil(const char* ssid, const char* password);
  void setup_wifi();

};
