#include <linux/if_ether.h>
#include <net/cfg80211.h>

#define IEEE80211_MAX_CHAINS 4

/* some backport stuff for mac80211 from cfg80211/... */
struct cfg80211_wowlan_wakeup {
	bool disconnect, magic_pkt, gtk_rekey_failure,
	     eap_identity_req, four_way_handshake,
	     rfkill_release, packet_80211,
	     tcp_match, tcp_connlost, tcp_nomoretokens;
	s32 pattern_idx;
	u32 packet_present_len, packet_len;
	const void *packet;
};

/* nl80211 */
struct wiphy_wowlan_tcp_support {
	const struct nl80211_wowlan_tcp_data_token_feature *tok;
	u32 data_payload_max;
	u32 data_interval_max;
	u32 wake_payload_max;
	bool seq;
};

struct cfg80211_wowlan_tcp {
	struct socket *sock;
	__be32 src, dst;
	u16 src_port, dst_port;
	u8 dst_mac[ETH_ALEN];
	int payload_len;
	const u8 *payload;
	struct nl80211_wowlan_tcp_data_seq payload_seq;
	u32 data_interval;
	u32 wake_len;
	const u8 *wake_data, *wake_mask;
	u32 tokens_size;
	/* must be last, variable member */
	struct nl80211_wowlan_tcp_data_token payload_tok;
};

#define MONITOR_FLAG_ACTIVE 0
#define NL80211_FEATURE_USERSPACE_MPM 0

static inline u32
ieee80211_mandatory_rates(struct ieee80211_supported_band *sband)
{
	struct ieee80211_rate *bitrates;
	u32 mandatory_rates = 0;
	enum ieee80211_rate_flags mandatory_flag;
	int i;

	if (WARN_ON(!sband))
		return 1;

	if (sband->band == IEEE80211_BAND_2GHZ)
		mandatory_flag = IEEE80211_RATE_MANDATORY_B;
	else
		mandatory_flag = IEEE80211_RATE_MANDATORY_A;

	bitrates = sband->bitrates;
	for (i = 0; i < sband->n_bitrates; i++)
		if (bitrates[i].flags & mandatory_flag)
			mandatory_rates |= BIT(i);
	return mandatory_rates;
}

static inline void netdev_attach_ops(struct net_device *dev,
				     const struct net_device_ops *ops)
{
	dev->netdev_ops = ops;
}

#define IEEE80211_RADIOTAP_MCS_STBC_SHIFT	5

static inline bool
ieee80211_operating_class_to_band(u8 operating_class,
				  enum ieee80211_band *band)
{
	switch (operating_class) {
	case 112:
	case 115 ... 127:
		*band = IEEE80211_BAND_5GHZ;
		return true;
	case 81:
	case 82:
	case 83:
	case 84:
		*band = IEEE80211_BAND_2GHZ;
		return true;
	case 180:
		*band = IEEE80211_BAND_60GHZ;
		return true;
	}

	/* stupid compiler */
	*band = IEEE80211_BAND_2GHZ;

	return false;
}

#define ASSOC_REQ_DISABLE_VHT 0

static inline void
cfg80211_report_wowlan_wakeup(struct wireless_dev *wdev,
			      struct cfg80211_wowlan_wakeup *wakeup, gfp_t gfp)
{
}

static inline void cfg80211_rx_unprot_mlme_mgmt(struct net_device *dev,
						void *data, int len)
{
	struct ieee80211_hdr *hdr = data;

	if (ieee80211_is_deauth(hdr->frame_control))
		cfg80211_send_unprot_deauth(dev, data, len);
	else
		cfg80211_send_unprot_disassoc(dev, data, len);
}

static inline void cfg80211_tx_mlme_mgmt(struct net_device *dev,
					 void *data, int len)
{
	struct ieee80211_hdr *hdr = data;

	if (ieee80211_is_deauth(hdr->frame_control))
		cfg80211_send_deauth(dev, data, len);
	else
		cfg80211_send_disassoc(dev, data, len);
}

static inline void cfg80211_rx_mlme_mgmt(struct net_device *dev,
					 void *data, int len)
{
	struct ieee80211_hdr *hdr = data;

	if (ieee80211_is_auth(hdr->frame_control))
		cfg80211_send_rx_auth(dev, data, len);
	else if (ieee80211_is_deauth(hdr->frame_control))
		cfg80211_send_deauth(dev, data, len);
	else
		cfg80211_send_disassoc(dev, data, len);
}

static inline void cfg80211_assoc_timeout(struct net_device *dev,
					  struct cfg80211_bss *bss)
{
	cfg80211_send_assoc_timeout(dev, bss->bssid);
}

static inline void cfg80211_auth_timeout(struct net_device *dev,
					 const u8 *bssid)
{
	cfg80211_send_auth_timeout(dev, bssid);
}

static inline void cfg80211_rx_assoc_resp(struct net_device *dev,
					  struct cfg80211_bss *bss,
					  void *data, int len)
{
	cfg80211_send_rx_assoc(dev, bss, data, len);
}

enum nl80211_bss_scan_width {
	NL80211_BSS_CHAN_WIDTH_20,
};

static inline int
ieee80211_chandef_max_power(struct cfg80211_chan_def *chandef)
{
	return chandef->chan->max_power;
}

static inline enum ieee80211_rate_flags
ieee80211_chandef_rate_flags(struct cfg80211_chan_def *chandef)
{
	return 0;
}

static inline enum nl80211_bss_scan_width
cfg80211_chandef_to_scan_width(const struct cfg80211_chan_def *chandef)
{
	return NL80211_BSS_CHAN_WIDTH_20;
}

static inline struct cfg80211_bss * __must_check
cfg80211_inform_bss_width_frame(struct wiphy *wiphy,
				struct ieee80211_channel *channel,
				enum nl80211_bss_scan_width scan_width,
				struct ieee80211_mgmt *mgmt, size_t len,
				s32 signal, gfp_t gfp)
{
	return cfg80211_inform_bss_frame(wiphy, channel, mgmt,
					 len, signal, gfp);
}

#define ieee80211_mandatory_rates(sband, width) ieee80211_mandatory_rates(sband)

#define IEEE80211_CHAN_HALF 0
#define IEEE80211_CHAN_QUARTER 0

static inline bool
iwl7000_cfg80211_rx_mgmt(struct wireless_dev *wdev, int freq, int sig_dbm,
			 const u8 *buf, size_t len, u32 flags, gfp_t gfp)
{
	return cfg80211_rx_mgmt(wdev, freq, sig_dbm, buf, len, gfp);
}
#define cfg80211_rx_mgmt iwl7000_cfg80211_rx_mgmt
