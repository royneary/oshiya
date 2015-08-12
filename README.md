#Oshiya
Oshyia is an app server for mobile XMPP clients as specified in [XEP-0357](http://xmpp.org/extensions/xep-0357.html). As such it receives push notification contents from those clients' XMPP servers and forwards them to the popular push notification services, e.g. APNS. It is implemented as XMPP component (see [XEP-0114](http://xmpp.org/extensions/xep-0114.html)).

Clients can register using adhoc commands. Oshiya aims to be compatible with [mod_push](https://github.com/royneary/mod_push) concerning commands and app server behaviour. This allows XEP-0357-compatible clients using mod_push's internal app server on an ejabberd server or, as an alternative, any XEP-0114-compatible server running Oshiya.

Oshiya is part of a GSoC 2015 project. Please send feedback.

##Features
Oshiya will support these push services:
 * [APNS (Apple push notification service)](https://developer.apple.com/library/ios/documentation/NetworkingInternet/Conceptual/RemoteNotificationsPG/Chapters/ApplePushService.html)
* [GCM (Google cloud messaging)](https://developers.google.com/cloud-messaging)
* [Mozilla SimplePush](https://wiki.mozilla.org/WebAPI/SimplePush)
* [Ubuntu Push](https://developer.ubuntu.com/en/start/platform/guides/push-notifications-client-guide)
* [WNS (Windows notification service)](https://msdn.microsoft.com/en-us//library/windows/apps/hh913756.aspx)

Currently only the GCM backend is usable. The Ubuntu backend is implemented but untested.

##Prerequisites
* libcurl 7.28.0 or later
* expat
* cmake 2.8 or later

##Installation
```bash
git clone https://github.com/royneary/oshiya.git
cd oshiya
git submodule init
git submodule update
mkdir build
cd build
# the config file path and the data storage location can be configured using cmake options
cmake -DCONFIG_FILE="/etc/oshiya/oshiya.yml" -DSTORAGE_DIR="/var/run/oshiya/" ..
make
# no automatic file copying à la make install yet
```

##Configuration
currently there's only an example config (mod_push's configuration is similar, see README.md over there):
```yaml
components:
  -
    host: "push.chatninja.org"
    server_host: "xmpp1.chatninja.org"
    port: 5237
    password: "ABCDEF123456"
    pubsub_host: "pubsub.chatninja.org"
    backends:
      -
        type: gcm
        certfile: "/etc/ssl/chatninja.pem"
        app_name: "chatninja"
        auth_key: "sdfF73HFk_fdhj8JFjfzqALkdj81dfjhs0jdEkf"
      -
        type: ubuntu
        certfile: "/etc/ssl/chatninja.pem"
        app_name: "any"
  -
    host: "apple-push.chatninja.org"
    server_host: "xmpp2.chatninja.org"
    port: 5238
    password: "POIUZT987654"
    pubsub_host: "pubsub.chatninja.org"
    backends:
      -
        type: apns
        certfile: "/etc/ssl/chatninja_apns.pem"
        app_name: "chatninja"
```

##Pubsub service configuration
The pubsub service is where the XMPP servers publish the push notification contents. It has the fulfill XEP-0357's requirements. Here is how ejabberd having mod_pubsub and mod_push installed can be configured:
```yaml
mod_pubsub:
  host : "pubsub.chatninja.org"
  nodetree : "tree"
  plugins:
    - "push"
```
