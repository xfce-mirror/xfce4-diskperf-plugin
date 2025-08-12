[![License](https://img.shields.io/badge/License-BSD%202--Clause-blue.svg)](https://gitlab.xfce.org/panel-plugins/xfce4-diskperf-plugin/-/blob/master/COPYING)

# xfce4-diskperf-plugin

Disk performance plugin for the Xfce4 panel. This plugin displays read&write bandwidth and utilization of storage devices and partitions based on data provided by Linux or a BSD-based system.

----

### Homepage

[DiskPerf documentation](https://docs.xfce.org/panel-plugins/xfce4-diskperf-plugin/start)

### Changelog

See [NEWS](https://gitlab.xfce.org/panel-plugins/xfce4-diskperf-plugin/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[DiskPerf source code](https://gitlab.xfce.org/panel-plugins/xfce4-diskperf-plugin)

### Download a Release Tarball

[DiskPerf archive](https://archive.xfce.org/src/panel-plugins/xfce4-diskperf-plugin/)
    or
[DiskPerf tags](https://gitlab.xfce.org/panel-plugins/xfce4-diskperf-plugin/-/tags)

### Installation

From source:

    % cd xfce4-diskperf-plugin
    % meson setup build
    % meson compile -C build
    % meson install -C build

From release tarball:

    % tar xf xfce4-diskperf-plugin-<version>.tar.xz
    % cd xfce4-diskperf-plugin-<version>
    % meson setup build
    % meson compile -C build
    % meson install -C build

### Uninstallation

    % ninja uninstall -C build

### Reporting Bugs

Visit the [reporting bugs](https://docs.xfce.org/panel-plugins/xfce4-diskperf-plugin/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.
