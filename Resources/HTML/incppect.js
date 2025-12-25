var incppect = {
    // websocket data
    ws: null,

    // default ws url - change to fit your needs
    ws_uri: 'ws://' + window.location.hostname + ':' + window.location.port + '/incppect',

    // vars data
    nvars: 0,
    vars_map: {},
    var_to_id: {},
    id_to_var: {},
    last_data: null,

    // requests data
    requests: [],
    requests_old: [],
    requests_new_vars: false,
    requests_regenerate: true,

    // timestamps
    t_start_ms: null,
    t_frame_begin_ms: null,
    t_requests_last_update_ms: null,

    // constants
    k_var_delim: ' ',
    k_auto_reconnect: true,
    k_requests_update_freq_ms: 50,

    // app-layer compression (gzip wrapper): "ICZ1" + uint32 original_size + gzip(payload)
    k_enable_app_compression: true,
    k_app_compress_min_bytes: 512,
    k_app_compress_level: 1,
    k_app_compression_magic: [73, 67, 90, 49], // 'I','C','Z','1'
    k_msg_type_compression_hello: 5,
    app_compression_active: false,
    app_compression_rx_seen: false,
    app_compression_tx_used: false,
    app_compression_tx_saved_bytes: 0,
    app_compression_rx_saved_bytes: 0,
    app_compression_tx_orig_bytes: 0,
    app_compression_tx_comp_bytes: 0,
    app_compression_rx_orig_bytes: 0,
    app_compression_rx_comp_bytes: 0,

    // rates (bytes per second)
    _rate_last_ms: null,
    _rate_last_client_tx: 0,
    _rate_last_client_rx: 0,
    _rate_last_server_tx: 0,
    _rate_last_server_rx: 0,
    rate_client_tx_Bps: 0,
    rate_client_rx_Bps: 0,
    rate_server_tx_Bps: 0,
    rate_server_rx_Bps: 0,

    // stats
    stats: {
        tx_n: 0,
        tx_bytes: 0,

        rx_n: 0,
        rx_bytes: 0,
    },

    // event handle
    event_handle : function(event_id, payload) {
        console.assert(false);
    },

    timestamp: function() {
        return window.performance && window.performance.now && window.performance.timing &&
        window.performance.timing.navigationStart ? window.performance.now() + window.performance.timing.navigationStart : Date.now();
    },

    init: function() {
        const onopen = this.onopen.bind(this);
        const onclose = this.onclose.bind(this);
        const onmessage = this.onmessage.bind(this);
        const onerror = this.onerror.bind(this);

        this.ws = new WebSocket(this.ws_uri);
        this.ws.binaryType = 'arraybuffer';
        this.ws.onopen = function(evt) { onopen(evt) };
        this.ws.onclose = function(evt) { onclose(evt) };
        this.ws.onmessage = function(evt) { onmessage(evt) };
        this.ws.onerror = function(evt) { onerror(evt) };

        this.t_start_ms = this.timestamp();
        this.t_requests_last_update_ms = this.timestamp() - this.k_requests_update_freq_ms;

        window.requestAnimationFrame(this.loop.bind(this));
    },

    has_streams: function() {
        return (typeof CompressionStream !== 'undefined') && (typeof DecompressionStream !== 'undefined');
    },

    update_rates: function(server_tx_total_bytes, server_rx_total_bytes) {
        const now = this.timestamp();
        if (this._rate_last_ms == null) {
            this._rate_last_ms = now;
            this._rate_last_client_tx = this.stats.tx_bytes || 0;
            this._rate_last_client_rx = this.stats.rx_bytes || 0;
            this._rate_last_server_tx = server_tx_total_bytes || 0;
            this._rate_last_server_rx = server_rx_total_bytes || 0;
            return;
        }

        const dt_ms = now - this._rate_last_ms;
        if (dt_ms < 500) return; // update ~2Hz

        const dt_s = dt_ms / 1000.0;

        const cur_client_tx = this.stats.tx_bytes || 0;
        const cur_client_rx = this.stats.rx_bytes || 0;
        const cur_server_tx = server_tx_total_bytes || 0;
        const cur_server_rx = server_rx_total_bytes || 0;

        this.rate_client_tx_Bps = (cur_client_tx - this._rate_last_client_tx) / dt_s;
        this.rate_client_rx_Bps = (cur_client_rx - this._rate_last_client_rx) / dt_s;
        this.rate_server_tx_Bps = (cur_server_tx - this._rate_last_server_tx) / dt_s;
        this.rate_server_rx_Bps = (cur_server_rx - this._rate_last_server_rx) / dt_s;

        this._rate_last_ms = now;
        this._rate_last_client_tx = cur_client_tx;
        this._rate_last_client_rx = cur_client_rx;
        this._rate_last_server_tx = cur_server_tx;
        this._rate_last_server_rx = cur_server_rx;
    },

    u8_from: function(typed) {
        if (typed instanceof Uint8Array) return typed;
        return new Uint8Array(typed.buffer, typed.byteOffset || 0, typed.byteLength || typed.length);
    },

    is_compressed_payload: function(u8) {
        if (u8.byteLength < 8) return false;
        const m = this.k_app_compression_magic;
        return u8[0] === m[0] && u8[1] === m[1] && u8[2] === m[2] && u8[3] === m[3];
    },

    stream_gzip: async function(u8) {
        const cs = new CompressionStream('gzip');
        const ab = await new Response(new Blob([u8]).stream().pipeThrough(cs)).arrayBuffer();
        return new Uint8Array(ab);
    },

    stream_ungzip: async function(u8) {
        const ds = new DecompressionStream('gzip');
        const ab = await new Response(new Blob([u8]).stream().pipeThrough(ds)).arrayBuffer();
        return new Uint8Array(ab);
    },

    decompress_payload: async function(u8) {
        // u8: [magic(4)][orig_len(4)][gzip_bytes...]
        const orig_len = (u8[4]) | (u8[5] << 8) | (u8[6] << 16) | (u8[7] << 24);
        const comp = u8.subarray(8);
        const dec = await this.stream_ungzip(comp);
        if (orig_len !== dec.byteLength) {
            console.warn("[incppect] decompressed size mismatch", orig_len, dec.byteLength);
        }
        return dec.buffer.slice(dec.byteOffset, dec.byteOffset + dec.byteLength);
    },

    maybe_compress_packet: async function(packet_u8) {
        // packet_u8 includes the 4-byte length prefix; we compress only the payload after it
        if (!this.k_enable_app_compression || !this.app_compression_active) return packet_u8;
        if (!this.has_streams()) return packet_u8;

        const payload = packet_u8.subarray(4);
        if (payload.byteLength < this.k_app_compress_min_bytes) return packet_u8;

        // NOTE: CompressionStream does not expose compression level; keep k_app_compress_level for symmetry only.
        const comp = await this.stream_gzip(payload);
        const wrapped_len = 8 + comp.byteLength;
        if (wrapped_len >= payload.byteLength) return packet_u8;

        const out = new Uint8Array(4 + wrapped_len);
        // length prefix = wrapped_len
        const num = wrapped_len;
        out[0] = num & 0xFF;
        out[1] = (num >> 8) & 0xFF;
        out[2] = (num >> 16) & 0xFF;
        out[3] = (num >> 24) & 0xFF;

        // magic
        const m = this.k_app_compression_magic;
        out[4] = m[0]; out[5] = m[1]; out[6] = m[2]; out[7] = m[3];
        // orig_len
        const ol = payload.byteLength >>> 0;
        out[8]  = ol & 0xFF;
        out[9]  = (ol >> 8) & 0xFF;
        out[10] = (ol >> 16) & 0xFF;
        out[11] = (ol >> 24) & 0xFF;
        // compressed bytes
        out.set(comp, 12);

        this.app_compression_tx_used = true;
        this.app_compression_tx_saved_bytes += (payload.byteLength - wrapped_len);
        this.app_compression_tx_orig_bytes += payload.byteLength;
        this.app_compression_tx_comp_bytes += wrapped_len;
        return out;
    },

    ws_send: function(packet) {
        // async send queue to keep ordering when compression is enabled
        if (!this._send_p) this._send_p = Promise.resolve();
        const u8 = this.u8_from(packet);
        this._send_p = this._send_p.then(async () => {
            const wire = await this.maybe_compress_packet(u8);
            this.ws.send(wire);
            this.stats.tx_n += 1;
            this.stats.tx_bytes += wire.byteLength;
        }).catch((e) => {
            console.error("[incppect] ws_send failed", e);
        });
    },

    loop: function() {
        if (this.ws == null) {
            if (this.k_auto_reconnect) {
                setTimeout(this.init.bind(this), 1000);
            }
            return;
        }

        if (this.ws.readyState !== this.ws.OPEN) {
            window.requestAnimationFrame(this.loop.bind(this));
            return;
        }

        this.t_frame_begin_ms = this.timestamp();
        this.requests_regenerate = this.t_frame_begin_ms - this.t_requests_last_update_ms > this.k_requests_update_freq_ms;

        if (this.requests_regenerate) {
            this.requests_old = this.requests;
            this.requests = [];
        }

        try {
            this.render();
        } catch(err) {
            this.onerror('Failed to render state: ' + err);
        }

        if (this.requests_regenerate) {
            if (this.requests_new_vars) {
                this.send_var_to_id_map();
                this.requests_new_vars = false;
            }
            this.send_requests();
            this.t_requests_last_update_ms = this.timestamp();
        }

        window.requestAnimationFrame(this.loop.bind(this));
    },

    get: function(path, ...args) {
        for (let i = 1; i < arguments.length; i++) {
            path = path.replace('%d', arguments[i]);
        }

        if (!(path in this.vars_map)) {
            this.vars_map[path] = new ArrayBuffer();
            this.var_to_id[path] = this.nvars;
            this.id_to_var[this.nvars] = path;
            ++this.nvars;

            this.requests_new_vars = true;
        }

        if (this.requests_regenerate) {
            this.requests.push(this.var_to_id[path]);
        }

        return this.vars_map[path];
    },

    get_abuf: function(path, ...args) {
        return this.get(path, ...args);
    },

    get_int8: function(path, ...args) {
        return this.get_int8_arr(path, ...args)[0];
    },

    get_int8_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Int8Array(abuf);
    },

    get_uint8: function(path, ...args) {
        return this.get_uint8_arr(path, ...args)[0];
    },

    get_uint8_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Uint8Array(abuf);
    },

    get_int16: function(path, ...args) {
        return this.get_int16_arr(path, ...args)[0];
    },

    get_int16_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Int16Array(abuf);
    },

    get_uint16: function(path, ...args) {
        return this.get_uint16_arr(path, ...args)[0];
    },

    get_uint16_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Uint16Array(abuf);
    },

    get_int32: function(path, ...args) {
        return this.get_int32_arr(path, ...args)[0];
    },

    get_int32_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Int32Array(abuf);
    },

    get_uint32: function(path, ...args) {
        return this.get_uint32_arr(path, ...args)[0];
    },

    get_uint32_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Uint32Array(abuf);
    },

    get_float: function(path, ...args) {
        return this.get_float_arr(path, ...args)[0];
    },

    get_float_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Float32Array(abuf);
    },

    get_double: function(path, ...args) {
        return this.get_double_arr(path, ...args)[0];
    },

    get_double_arr: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return new Float64Array(abuf);
    },

    abut_to_str: function(abuf) {
        let enc = new TextDecoder("utf-8");
        return enc.decode(new Uint8Array(abuf));
    },

    get_str: function(path, ...args) {
        const abuf = this.get(path, ...args);
        return this.abut_to_str(abuf);
    },

    set_data_num: function (data, num) {
        data[0] = num & 0xFF;
        data[1] = (num >> 8) & 0xFF;
        data[2] = (num >> 16) & 0xFF;
        data[3] = (num >> 24) & 0xFF;
    },

    send: function(msg) {
        const enc_msg = new TextEncoder().encode(msg);
        const data = new Int8Array(8 + enc_msg.length + 1);
        this.set_data_num(data, data.length - 4);
        data[4] = 4;
        data.set(enc_msg, 8);
        data[8 + enc_msg.length] = 0;
        this.ws_send(data);
    },

    send_var_to_id_map: function() {
        let msg = '';
        const delim = this.k_var_delim;
        for (const key in this.var_to_id) {
            let nidxs = 0;
            let idxs = delim;
            const keyp = key.replace(/\[-?\d*\]/g, function (m) {
                ++nidxs;
                idxs += m.replace(/[\[\]]/g, '') + delim;
                return '[%d]';
            });
            msg += keyp + delim + this.var_to_id[key].toString() + delim + nidxs + idxs;
        }
        const data = new Int8Array(8 + msg.length + 1);
        const enc = new TextEncoder();
        this.set_data_num(data, data.length - 4);
        data[4] = 1;
        data.set(enc.encode(msg), 8);
        data[8 + msg.length] = 0;
        this.ws_send(data);
    },

    send_requests: function() {
        let same = true;
        if (this.requests_old === null || this.requests.length !== this.requests_old.length){
            same = false;
        } else {
            for (let i = 0; i < this.requests.length; ++i) {
                if (this.requests[i] !== this.requests_old[i]) {
                    same = false;
                    break;
                }
            }
        }

        if (same) {
            let data = new Int32Array(2);
            data[0] = 4;
            data[1] = 3;
            this.ws_send(data);
        } else {
            let data = new Int32Array(2 + this.requests.length);
            data[0] = data.length * 4 - 4;
            data[1] = 2;
            data.set(new Int32Array(this.requests), 2);
            this.ws_send(data);
        }
    },

    onopen: function(evt) {
        // Handshake / negotiation: only enable server->client compression after we tell the server we can decompress.
        const can = this.k_enable_app_compression && this.has_streams();
        if (can) {
            // Send hello uncompressed (uses Int32 protocol used elsewhere):
            // [len=8][type=5][version=1]
            const hello = new Int32Array(3);
            hello[0] = 8;
            hello[1] = this.k_msg_type_compression_hello;
            hello[2] = 1;
            this.ws.send(hello);
            this.stats.tx_n += 1;
            this.stats.tx_bytes += hello.byteLength;
        }
        this.app_compression_active = can;
        this.app_compression_rx_seen = false;
        this.app_compression_tx_used = false;
        this.app_compression_tx_saved_bytes = 0;
        this.app_compression_rx_saved_bytes = 0;
        this.app_compression_tx_orig_bytes = 0;
        this.app_compression_tx_comp_bytes = 0;
        this.app_compression_rx_orig_bytes = 0;
        this.app_compression_rx_comp_bytes = 0;
    },

    onclose: function(evt) {
        this.nvars = 0;
        this.vars_map = {};
        this.var_to_id = {};
        this.id_to_var = {};
        this.requests = null;
        this.requests_old = null;
        this.ws = null;

        // reset rates
        this._rate_last_ms = null;
        this.rate_client_tx_Bps = 0;
        this.rate_client_rx_Bps = 0;
        this.rate_server_tx_Bps = 0;
        this.rate_server_rx_Bps = 0;
    },

    onmessage: function(evt) {
        // Receive queue to preserve in-order processing even when decompression is async.
        if (!this._recv_p) this._recv_p = Promise.resolve();

        // Snapshot payload now (avoid relying on evt after async boundary)
        const msg_ab = evt.data;
        const msg_len = msg_ab ? msg_ab.byteLength : 0;

        this._recv_p = this._recv_p.then(async () => {
            this.stats.rx_n += 1;
            this.stats.rx_bytes += msg_len;

            let data_ab = msg_ab;
            // server->client frames may be wrapped with app-layer compression; unwrap before parsing
            if (this.has_streams()) {
                const u8 = new Uint8Array(data_ab);
                if (this.is_compressed_payload(u8)) {
                    this.app_compression_rx_seen = true;
                    // saved = original_size - received_compressed_size (including ICZ1 header)
                    const orig_len = (u8[4]) | (u8[5] << 8) | (u8[6] << 16) | (u8[7] << 24);
                    const saved = (orig_len >>> 0) - (u8.byteLength >>> 0);
                    if (saved > 0) this.app_compression_rx_saved_bytes += saved;
                    this.app_compression_rx_orig_bytes += (orig_len >>> 0);
                    this.app_compression_rx_comp_bytes += (u8.byteLength >>> 0);
                    data_ab = await this.decompress_payload(u8);
                }
            }

            const type_all = (new Uint32Array(data_ab))[0];

            if (this.last_data != null && type_all === 1) {
                const ntotal = data_ab.byteLength / 4 - 1;

                const src_view = new Uint32Array(data_ab, 4);
                const dst_view = new Uint32Array(this.last_data, 4);

                let k = 0;
                for (let i = 0; i < ntotal/2; ++i) {
                    const n = src_view[2*i + 0];
                    const c = src_view[2*i + 1];
                    for (let j = 0; j < n; ++j) {
                        dst_view[k] = dst_view[k] ^ c;
                        ++k;
                    }
                }
            } else {
                this.last_data = data_ab;
            }

            const int_view = new Uint32Array(this.last_data);
            let offset = 1;
            let offset_new = 0;
            const total_size = this.last_data.byteLength;
            let id = 0;
            let type = 0;
            let len = 0;
            while (4*offset < total_size) {
                type = int_view[offset];
                id = int_view[offset + 1];
                len = int_view[offset + 2];
                if ((len / 4) % 1 !== 0)
                {
                    break;
                }
                offset += 3;
                offset_new = offset + len/4;
                if (type === 0) {
                    this.vars_map[this.id_to_var[id]] = this.last_data.slice(4*offset, 4*offset_new);
                }
                else if (type === 1) {
                    const src_view = new Uint32Array(this.last_data, 4 * offset);
                    const dst_view = new Uint32Array(this.vars_map[this.id_to_var[id]]);

                    let k = 0;
                    for (let i = 0; i < len/8; ++i) {
                        const n = src_view[2 * i];
                        const c = src_view[2 * i + 1];
                        for (let j = 0; j < n; ++j) {
                            dst_view[k] = dst_view[k] ^ c;
                            ++k;
                        }
                    }
                }
                else if (type === 2) {
                    this.event_handle(id, this.last_data.slice(4*offset, 4*offset_new));
                }
                else {
                    console.assert(false);
                }
                offset = offset_new;
            }
        }).catch((e) => {
            console.error("[incppect] onmessage failed", e);
        });
    },

    onerror: function(evt) {
        console.error("[incppect]", evt);
    },

    render: function() {
    },
}