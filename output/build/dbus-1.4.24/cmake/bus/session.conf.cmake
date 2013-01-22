<!-- This configuration file controls the per-user-login-session message bus.
     Add a session-local.conf and edit that rather than changing this 
     file directly. -->

<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <!-- Our well-known bus type, don't change this -->
  <type>session</type>

  <listen>@DBUS_SESSION_BUS_DEFAULT_ADDRESS@</listen>

  <standard_session_servicedirs />

  <policy context="default">
    <!-- Allow everything to be sent -->
    <allow send_destination="*"/>
    <!-- Allow everything to be received -->
    <allow eavesdrop="true"/>
    <!-- Allow anyone to own anything -->
    <allow own="*"/>
  </policy>

  <!-- This is included last so local configuration can override what's 
       in this standard file -->
  <include ignore_missing="yes">session-local.conf</include>

  <include if_selinux_enabled="yes" selinux_root_relative="yes">contexts/dbus_contexts</include>

</busconfig>
