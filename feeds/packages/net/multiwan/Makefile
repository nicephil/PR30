#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=multiwan
PKG_VERSION:=1.0.12
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/multiwan
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+ip +iptables +iptables-utils +iptables-mod-conntrack +iptables-mod-conntrack-extra +iptables-mod-ipopt
  TITLE:=Simple multi WAN configuration
  URL:=ftp://ftp.netlab7.com/
  MAINTAINER:=Craig M. Coffee <craigc@netlab7.com>
endef

define Package/multiwan/description
An agent script that makes Multi-WAN configuration simple,
easy and manageable. Complete with load balancing, failover and an easy
to manage traffic ruleset. Allows for configuration of up to 9 wan links.
endef

define Package/multiwan/conffiles
/etc/config/multiwan
endef

define Build/Compile
endef

define Package/multiwan/install
	$(CP) ./files/* $(1)
endef

$(eval $(call BuildPackage,multiwan))
