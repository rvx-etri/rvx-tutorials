## ****************************************************************************
## ****************************************************************************
## Copyright SoC Design Research Group, All rights reserved.    
## Electronics and Telecommunications Research Institute (ETRI)
##
## THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE 
## WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS 
## TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE 
## REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL 
## SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE, 
## IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE 
## COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
## ****************************************************************************
## 2021-05-21
## Kyuseung Han (han@etri.re.kr)
## ****************************************************************************
## ****************************************************************************

from pathlib import Path

from config_file_manager import *
from rvx_config import *

class RvxAppConfig(ConfigFileManager):
  pass

class RvxWifiConfig(RvxAppConfig):
  def __init__(self, rvx_config:RvxConfig, dir_path:Path):
    super().__init__('rvx_wifi_config', dir_path / rvx_config.wifi_config_filename, rvx_config.key)
    #self.allowed_set = frozenset(('wifi_ssid','wifi_security_type','wifi_passwd'))
    self.allowed_set = frozenset(('wifi_ssid','wifi_passwd'))
    if not self.check(self.allowed_set, exact=True):
      self.clear()
      assert self.check(self.allowed_set, exact=True)

  def clear(self):
    super().clear()
    self.set_attr('wifi_ssid', None)
    #self.set_attr('wifi_security_type', None)
    self.set_attr('wifi_passwd', None)

  def set_from_input(self):
    self.set_attr_from_input('wifi_ssid', 'WIFI SSID')
    #self.set_attr_from_input('wifi_security_type', 'WIFI security type', ('WEP','WPA/WPA2-Personal','WPA/WPA2-Enterprise'))
    self.set_attr_from_input('wifi_passwd', 'WIFI password')

class RvxAixConfig(ConfigFileManager):
  def __init__(self, rvx_config:RvxConfig, dir_path:Path):
    super().__init__('rvx_aix_config', dir_path / rvx_config.aix_config_filename, rvx_config.key)
    self.allowed_set = frozenset(('aix_ip_address', 'aix_tcp_port', 'aix_username', 'aix_passwd'))
    if not self.check(self.allowed_set, exact=True):
      self.clear()
      assert self.check(self.allowed_set, exact=True)

  def clear(self):
    super().clear()
    self.set_attr('aix_ip_address', None)
    self.set_attr('aix_tcp_port', None)
    self.set_attr('aix_username', None)
    self.set_attr('aix_passwd', None)

  def set_from_input(self):
    self.set_attr_from_input('aix_ip_address', 'AIX server IP address')
    self.set_attr_from_input('aix_tcp_port', 'AIX server TCP port', 'int')
    self.set_attr_from_input('aix_username', 'AIX server username')
    self.set_attr_from_input('aix_passwd', 'AIX server password', None, True)
