/* Opacity Windows — KWin script (watcher).
 *
 * Pushes the current list of normal-window captions to the Ghosty
 * tray app over D-Bus whenever the window list changes, so the app can offer
 * per-window transparency overrides.
 *
 * Captions are the only identity shared with the effect.
 */

/*global workspace, callDBus, print */

var SERVICE = "org.kde.opacity-slider";
var PATH = "/";
var IFACE = "org.kde.opacityslider";

function pushList() {
    var names = [];
    var wins = workspace.windowList();
    for (var i = 0; i < wins.length; i++) {
        var w = wins[i];
        if (w.managed && w.normalWindow && w.caption) {
            names.push(String(w.caption));
        }
    }
    callDBus(SERVICE, PATH, IFACE, "ApplyWindows", names);
}

pushList();
workspace.windowAdded.connect(pushList);
workspace.windowRemoved.connect(pushList);

print("opacity-windows watcher armed");