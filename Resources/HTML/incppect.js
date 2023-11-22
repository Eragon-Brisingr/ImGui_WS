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
        var onopen = this.onopen.bind(this);
        var onclose = this.onclose.bind(this);
        var onmessage = this.onmessage.bind(this);
        var onerror = this.onerror.bind(this);

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
        for (var i = 1; i < arguments.length; i++) {
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
        var abuf = this.get(path, ...args);
        return new Int8Array(abuf);
    },

    get_uint8: function(path, ...args) {
        return this.get_uint8_arr(path, ...args)[0];
    },

    get_uint8_arr: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return new Uint8Array(abuf);
    },

    get_int16: function(path, ...args) {
        return this.get_int16_arr(path, ...args)[0];
    },

    get_int16_arr: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return new Int16Array(abuf);
    },

    get_uint16: function(path, ...args) {
        return this.get_uint16_arr(path, ...args)[0];
    },

    get_uint16_arr: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return new Uint16Array(abuf);
    },

    get_int32: function(path, ...args) {
        return this.get_int32_arr(path, ...args)[0];
    },

    get_int32_arr: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return new Int32Array(abuf);
    },

    get_uint32: function(path, ...args) {
        return this.get_uint32_arr(path, ...args)[0];
    },

    get_uint32_arr: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return new Uint32Array(abuf);
    },

    get_float: function(path, ...args) {
        return this.get_float_arr(path, ...args)[0];
    },

    get_float_arr: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return new Float32Array(abuf);
    },

    get_double: function(path, ...args) {
        return this.get_double_arr(path, ...args)[0];
    },

    get_double_arr: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return new Float64Array(abuf);
    },

    abut_to_str: function (abuf) {
        let enc = new TextDecoder("utf-8");
        return enc.decode(new Uint8Array(abuf));
    },

    get_str: function(path, ...args) {
        var abuf = this.get(path, ...args);
        return this.abut_to_str(abuf);
    },

    set_data_num: function (data, num) {
        data[0] = num & 0xFF;
        data[1] = (num >> 8) & 0xFF;
        data[2] = (num >> 16) & 0xFF;
        data[3] = (num >> 24) & 0xFF;
    },

    send: function(msg) {
        var enc_msg = new TextEncoder().encode(msg);
        var data = new Int8Array(8 + enc_msg.length + 1);
        this.set_data_num(data, data.length - 4);
        data[4] = 4;
        data.set(enc_msg, 8);
        data[8 + enc_msg.length] = 0;
        this.ws.send(data);

        this.stats.tx_n += 1;
        this.stats.tx_bytes += data.length;
    },

    send_var_to_id_map: function() {
        var msg = '';
        var delim = this.k_var_delim;
        for (var key in this.var_to_id) {
            var nidxs = 0;
            var idxs = delim;
            var keyp = key.replace(/\[-?\d*\]/g, function(m) { ++nidxs; idxs += m.replace(/[\[\]]/g, '') + delim; return '[%d]'; });
            msg += keyp + delim + this.var_to_id[key].toString() + delim + nidxs + idxs;
        }
        var data = new Int8Array(8 + msg.length + 1);
        var enc = new TextEncoder();
        this.set_data_num(data, data.length - 4);
        data[4] = 1;
        data.set(enc.encode(msg), 8);
        data[8 + msg.length] = 0;
        this.ws.send(data);

        this.stats.tx_n += 1;
        this.stats.tx_bytes += data.length;
    },

    send_requests: function() {
        var same = true;
        if (this.requests_old === null || this.requests.length !== this.requests_old.length){
            same = false;
        } else {
            for (var i = 0; i < this.requests.length; ++i) {
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
            this.ws.send(data);

            this.stats.tx_n += 1;
            this.stats.tx_bytes += data.length;
        } else {
            let data = new Int32Array(2 + this.requests.length);
            data[0] = data.length * 4 - 4;
            data[1] = 2;
            data.set(new Int32Array(this.requests), 2);
            this.ws.send(data);

            this.stats.tx_n += 1;
            this.stats.tx_bytes += data.length;
        }
    },

    onopen: function(evt) {
    },

    onclose: function(evt) {
        this.nvars = 0;
        this.vars_map = {};
        this.var_to_id = {};
        this.id_to_var = {};
        this.requests = null;
        this.requests_old = null;
        this.ws = null;
    },

    onmessage: function(evt) {
        this.stats.rx_n += 1;
        this.stats.rx_bytes += evt.data.byteLength;

        const type_all = (new Uint32Array(evt.data))[0];

        if (this.last_data != null && type_all === 1) {
            const ntotal = evt.data.byteLength / 4 - 1;

            const src_view = new Uint32Array(evt.data, 4);
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
            this.last_data = evt.data;
        }

        const int_view = new Uint32Array(this.last_data);
        let offset = 1;
        let offset_new = 0;
        const total_size = this.last_data.byteLength;
        let id = 0;
        let type = 0;
        let len = 0;
        while (4*offset < total_size) {
            id = int_view[offset + 0];
            type = int_view[offset + 1];
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
                    const n = src_view[2 * i + 0];
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
    },

    onerror: function(evt) {
        console.error("[incppect]", evt);
    },

    render: function() {
    },
}