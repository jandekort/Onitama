// Onitama engine host. Runs the WASM engine off the main thread so its
// searches never freeze the UI. Speaks a tiny RPC protocol with the page:
//   in : { id, method, args }   -> calls the matching engine function
//   out: { id, result }         -> the return value for that request
// plus a one-off { type: "ready" } once the engine has initialized.

let api = null;

// Emscripten reads this global during startup; define it before loading the
// engine so onRuntimeInitialized fires once the WASM is ready.
var Module = {
  onRuntimeInitialized: function () {
    try {
      api = {
        newGame:   Module.cwrap("og_new_game", null,     ["number", "number"]),
        state:     Module.cwrap("og_state",    "string", []),
        play:      Module.cwrap("og_play",     "number", ["number"]),
        aiMove:    Module.cwrap("og_ai_move",  "number", ["number", "number", "number"]),
        undo:      Module.cwrap("og_undo",     "number", []),
        evalScore:   Module.cwrap("og_eval",         "number", ["number", "number", "number", "number"]),
        engineLines: Module.cwrap("og_engine_lines", "string", []),
        searchInfo:  Module.cwrap("og_search_info",  "string", []),
        bestLine:    Module.cwrap("og_best_line",    "string", ["number", "number", "number", "string"]),
      };

      // Try to add state encoding/decoding
      try {
        console.log("Module._og_encode_state exists:", typeof Module._og_encode_state);
        api.encodeState = Module.cwrap("og_encode_state", "string", []);
        console.log("encodeState wrapped successfully");
      } catch (e) {
        console.error("og_encode_state wrapping failed:", e.message, e.stack);
      }
      try {
        console.log("Module._og_decode_state exists:", typeof Module._og_decode_state);
        api.decodeState = Module.cwrap("og_decode_state", "number", ["string"]);
        console.log("decodeState wrapped successfully");
      } catch (e) {
        console.error("og_decode_state wrapping failed:", e.message, e.stack);
      }

      // Try to add findPuzzle if available
      try {
        api.findPuzzle = Module.cwrap("og_find_puzzle", "string", ["number", "number", "number"]);
      } catch (e) {
        console.warn("og_find_puzzle not available:", e.message);
      }
      postMessage({ type: "ready" });
    } catch (e) {
      postMessage({ type: "error", message: "Failed to initialize: " + e.toString() });
    }
  }
};

// Resolve the script URL relative to the worker location, not the page. Carry
// the page's cache-buster (?v=...) through to onitama.js so a rebuilt wasm is
// always picked up — workers cache importScripts aggressively, independently of
// a page refresh. Reusing the page's version (rather than a fresh timestamp)
// keeps the wasm cached within a session, so respawning the worker is cheap.
try {
  const v = new URLSearchParams(self.location.search).get("v");
  const scriptUrl = new URL("onitama.js" + (v ? "?v=" + v : ""), self.location.href).href;
  importScripts(scriptUrl);
} catch (e) {
  postMessage({ type: "error", message: "Failed to load onitama.js: " + e.toString() });
}

onmessage = function (e) {
  const { id, method, args } = e.data;
  try {
    if (!api || !api[method]) {
      postMessage({ id, error: "Method not found: " + method });
      return;
    }
    const result = api[method].apply(null, args || []);
    postMessage({ id, result });
  } catch (err) {
    postMessage({ id, error: err.toString() });
  }
};
