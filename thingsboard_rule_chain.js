var rr_buffer_size = msg.rr_buffer.length;

//Diagnose
var sum_rr = 0;
var sum_rr_sqr = 0;
var sum_rr_diff_sqr = 0;
var valid_rr_count = 0;

var prev_rr = msg.rr_buffer[0];
for (var i = 0; i < rr_buffer_size; i++) {
    var rr = msg.rr_buffer[i];
    valid_rr_count += (rr) ? 1 : 0;
    sum_rr += rr;
    sum_rr_sqr += (rr * rr);

    if (i > 0) {
        var diff = rr - prev_rr;
        sum_rr_diff_sqr += (diff * diff);
        prev_rr = rr;
    }
}
//SDNN = sqrt(mean_rr_sqr - mean_rr^2) (Variance = E[X^2] - (E[X])^2)
var mean_rr = sum_rr / rr_buffer_size;
var mean_rr_sqr = sum_rr_sqr / rr_buffer_size;

var hr = (mean_rr !== 0) ? (60000 / mean_rr) : 0;
var sdnn = Math.sqrt(mean_rr_sqr - (mean_rr * mean_rr));
var rmssd = Math.sqrt(sum_rr_diff_sqr / (rr_buffer_size - 1));
var cvrr = sdnn / mean_rr;

//casting to interger
hr = parseInt(hr) || 0;
sdnn = parseInt(sdnn) || 0;
rmssd = parseInt(rmssd) || 0;

var progress = (valid_rr_count / rr_buffer_size) * 100;
var status_HR = "Normal";
var status_AFib = "Low AFib risk";
var status_Stress = "Relaxed";
var status_L0_PLUS = "Normal";
var status_L0_MINUS = "Normal";

if(msg.L0_PLUS == 1) status_L0_PLUS = "Lead off detected !"
if(msg.L0_MINUS == 1) status_L0_MINUS = "Lead off detected !"
// --- 2. DIAGNOSIS LOGIC ---

// --- HR ---
if (hr > 100) status_HR = "Tachycardia";
else if (hr < 50) status_HR = "Bradycardia";

// --- Stress ---
if (rmssd < 12)
    status_Stress = "High stress (low HRV)";
else if (rmssd < 27)
    status_Stress = "Moderate stress";

// --- AFib ---
if (rmssd > 80 && sdnn > 80 && cvrr >= 0.15 && cvrr <=0.35)
    status_AFib = "Possible AFib (RR irregularity)";


return {
    msg: {
        HR: hr,
        SDNN: sdnn,
        RMSSD: rmssd,
        ecg_buffer: msg.ecg_buffer,
        status_HR: status_HR,
        status_AFib: status_AFib,
        status_Stress: status_Stress,
        status_L0_MINUS: status_L0_MINUS,
        status_L0_PLUS: status_L0_PLUS,
        progress: progress,
    },
    metadata: metadata,
    msgType: msgType,
    ts: msg.time
};