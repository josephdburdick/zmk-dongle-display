import importlib
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
gen = importlib.import_module("gen_eight_ball")


def test_twelve_distinct_frames_of_40px():
    frames = [gen.render_frame(i) for i in range(12)]
    assert len(frames) == 12
    for f in frames:
        assert f.size == (40, 40)
        assert f.mode == "1"
    for a, b in zip(frames, frames[1:] + frames[:1]):
        assert a.tobytes() != b.tobytes()


def test_frame_is_deterministic():
    assert gen.render_frame(3).tobytes() == gen.render_frame(3).tobytes()


def test_ball_outline_is_lit_and_body_is_dark():
    f = gen.render_frame(0)
    # topmost pixel of the outline ring on the center column is lit
    assert f.getpixel((20, 1)) == 255
    # a body pixel just inside the ring, away from the disc, is dark
    assert f.getpixel((4, 20)) == 0


def test_i1_encoding_has_palette_and_208_bytes():
    data = gen.frame_to_i1_bytes(gen.render_frame(0))
    assert len(data) == 208
    assert data[:8] == bytes([0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF])


def test_c_file_declares_all_frames(tmp_path):
    out = tmp_path / "eight_ball_images.c"
    gen.write_c_file(out, [gen.render_frame(i) for i in range(12)])
    text = out.read_text()
    for i in range(12):
        assert f"const lv_img_dsc_t eight_ball_{i:02d} = {{" in text
    assert text.count(".header.cf = LV_COLOR_FORMAT_I1") == 12
    assert ".data_size = 208" in text
