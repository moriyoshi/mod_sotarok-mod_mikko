<?php
require 'Xlib.php';
$fonts = array(
    'hiragino-mincho' => "/System/Library/Fonts/ヒラギノ明朝 ProN W6.otf",
    'courier'         => "/Library/Fonts/Courier New Bold.ttf",
);

$texts = array(
    array("はい"),
    array("このたびは"),
    array("sotarokくん\n"),
    array("sotarokくん\nmikkoさん"),
    array("おめでとうございます"),
    array("自己紹介"),
    array("PHP"),
    array("PHP=\n"),
    array("PHP=\nPeace and\nHappiness\nthrough Prosperity"),
    array("さて"),
    array("OSSコミュニティーには"),
    array("開発者の結婚の際に"),
    array("新郎と新婦の"),
    array("オリジナル\n○○○○○○○○○○"),
    array("を作って、\n贈呈するという慣習が\nあります。"),
    array("オリジナル\n○○○○○○○○○○"),
    array("オリジナル\nApacheモジュール"),
    array("というわけで"),
    array("こちらに"),
    array("mod_sotarok.so"),
    array("mod_mikko.so"),
    array("をご用意いたしました。"),
    array(""),
);

ini_set('memory_limit', -1);

$x = XDisplay::create();
$bc = $x->allocColor($x->screens[0]->colormap, 0xffff, 0xffff, 0xffff);
$wnd = $x->createWindow($x->screens[0]->rootWindow,
        0, 0, $x->screens[0]->width, $x->screens[0]->height, 0, XClient::InputOutput, null,
        array('backgroundPixel' => $bc['pixel'],
              'eventMask' => XClient::ExposureMask
                             | XClient::ButtonPressMask
                             | XClient::KeyPressMask
                             | XClient::StructureNotifyMask,
              'overrideRedirect' => true));
$_motif_wm_hints = $x->internAtom("_MOTIF_WM_HINTS");
$wnd->changeProperty(XClient::PropModeReplace, $_motif_wm_hints, $_motif_wm_hints, 32, "\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

$gc = $wnd->createGC(
        array('function' => 3, 'lineWidth' => 8, 'lineStyle' => 0));
$x->mapWindow($wnd);

class Canvas {
    protected $im;
    public $width;
    public $height;
    protected $backgroundColor;
    protected $foregroundColor;

    public function __construct($width, $height) {
        $this->im = imagecreatetruecolor($width, $height);
        $this->width = $width;
        $this->height = $height;
        $this->backgroundColor = imagecolorallocate($this->im, 255, 255, 255);
        $this->foregroundColor = imagecolorallocate($this->im, 0, 0, 0);
    }

    public function toPixels() {
        ob_start();
        imagegd2($this->im, NULL, 64);
        $data = substr(ob_get_clean(), 23);
        $data |= substr($data & str_repeat("\x00\x00\x00\xff", strlen($data) / 4), 3);
        return $data;
    }

    public function clear() {
        imagefilledrectangle($this->im, 0, 0, $this->width, $this->height, $this->backgroundColor);
    }

    public function setFont($font) {
        $this->font = $font;
    }

    public function drawText($text, $sz) {
        for (;;) {
            $bbox = imagettfbbox($sz, 0, $this->font, $text);
            if ($bbox[2] < $this->width * 0.8)
                break;
            $sz -= 8;
        }
        imagettftext($this->im, $sz, 0, ($this->width - $bbox[2]) / 2, ($this->height - $bbox[3]) / 2 + $sz * 0.6, $this->foregroundColor, $this->font, $text);
    }
}
     
$cv = new Canvas($x->screens[0]->width, $x->screens[0]->height);

function drawImage($wnd, $gc, $x, $y, $width, $height, $data) {
    $ncx = (int)($width / 64);
    $ncy = (int)($height / 64);
    for ($j = 0; $j < $ncy; $j++) {
        $ch = min(64, $height - $j * 64);
        for ($i = 0; $i < $ncx; $i++) {
            $cw = min(64, $width - $i * 64);
            $wnd->putImage(2, $gc, $cw, $ch, $x + $i * 64, $y + $j * 64, 0, 24, substr($data, $i * 16384 + $j * $width * 256, 16384));
        }
    }
}

$data = null;
$counter = 0;
for (;;) {
    switch ($x->nextEvent($ev)) {
    case XClient::Expose:
        if ($data !== null)
            drawImage($wnd, $gc, 0, 0, $cv->width, $cv->height, $data);
        break;
    case XClient::ButtonPress:
    case XClient::KeyPress:
    case XClient::KeyRelease:
        $cv->clear();
        $line = array_shift($texts);
        if ($line) {
            $text = $line[0];
            $fontname = 'hiragino-mincho';
            if (isset($line[1])) {
                if (isset($line[1]['font']))
                    $fontname = $line[1]['font'];
            }
            $cv->setFont($fonts[$fontname]);
            $cv->drawText($text, 84);
        }
        $data = $cv->toPixels();
        drawImage($wnd, $gc, 0, 0, $cv->width, $cv->height, $data);
        break;
    case XClient::MotionNotify:
        break;
    }
}
