$(document).ready(function () {

    if ($('#inline').length) {
        var x = $('#inline').minicolors({
            inline: $('#inline').attr('data-inline') === 'true',
            theme: 'bootstrap'
        });
    }

    if ($('#currentDateTime').length) {
        window.setInterval(showDateTime, 900);
    }
});

function showDateTime() {
    console.log(1);
    var currentDateTime = $('#currentDateTime').get(0);
    currentDateTime.innerHTML = moment().format('MMMM MM YYYY HH:mm:ss');
}

function setColor() {
    var color = $('#inline').minicolors('rgbObject');
    console.log(color);
    $.post("/color", color);
}

function setBrightness() {
    var brightness = $('#rangeBrightness').val();
    console.log(brightness);
    $.post("/brightness", { brightness: brightness });
}

function setDateTime() {
    var d = moment().format('MMMM MM YYYY');
    var t = moment().format('HH:mm:ss');
    var datetime = { date: d, time: t };
    console.log(datetime);
    $.post("/setdate", datetime);
}

function startCountdown() {
    var color = $('#inline').minicolors('rgbObject');
    var h = $('#hours').val();
    var m = $('#minutes').val();
    var s = $('#seconds').val();

    if (isNaN(h) || isNaN(m) || isNaN(s)){
        $('#validNrs').show();
        return;
    } else {
        $('#validNrs').hide();
    }   

    var ms = (h * 60 * 60 * 1000) + (m * 60 * 1000) + (s * 1000);

    var countdown = { ms: ms, r: color.r, g: color.g, b: color.b};
    console.log(countdown);

    $.post("/countdown", countdown);
}

function showTemperature(){
    var c = $('#temperatureCorrection').val();
    var d = $('#temperatureDisplay').val();
    var temperature = { correction: c, symbol: d };
    $.post("/temperature", temperature);
}

function showClock() {
    $.post("/clock");
}