// Puzzle generator worker (no game engine)
// RPC protocol: { id, method, args } -> { id, result } or { id, error }

let api = null;
let loadError = null;

// Set up Module BEFORE loading the WASM so it can use our callbacks
self.Module = {
  onRuntimeInitialized: function () {
    console.log("[puzzle-worker] onRuntimeInitialized called");
    try {
      api = {};
      if (!self.Module.cwrap) {
        throw new Error("Module.cwrap not available - WASM module may not have loaded correctly");
      }
      api.find = self.Module.cwrap("puzzle_find", "string", ["number", "number", "number", "number"]);
      console.log("[puzzle-worker] puzzle_find initialized");
      postMessage({ type: "ready" });
    } catch (e) {
      const errorMsg = e && e.message ? e.message : e.toString();
      console.error("[puzzle-worker] Initialization failed:", errorMsg);
      postMessage({ type: "error", message: "Failed to initialize: " + errorMsg });
    }
  }
};

// Load the WASM module
(function() {
  try {
    const baseUrl = self.location.href;
    console.log("[puzzle-worker] Worker location:", baseUrl);
    const scriptUrl = new URL("puzzle-engine.js?v=" + Date.now(), baseUrl).href;
    console.log("[puzzle-worker] Loading WASM from:", scriptUrl);

    importScripts(scriptUrl);
    console.log("[puzzle-worker] WASM loaded and script executed");
  } catch (e) {
    loadError = e;
    const errorMsg = e && e.message ? e.message : (e ? e.toString() : "Unknown error");
    console.error("[puzzle-worker] Failed to load WASM:", errorMsg, e);
    postMessage({
      type: "error",
      message: "importScripts failed: " + errorMsg
    });
  }
})();

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
