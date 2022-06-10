    .global font
    .global font_size
    .global style
    .global style_size
    .global tw2s
    .global tw2s_size
    .global TSPhrases
    .global TSPhrases_size
    .global TWVariantsRevPhrases
    .global TWVariantsRevPhrases_size
    .global TWVariantsRev
    .global TWVariantsRev_size
    .global TSCharacters
    .global TSCharacters_size
    .global geetest_js
    .global geetest_js_size
    .global index_html
    .global index_html_size
    .section .rodata

font:
    .incbin "SourceHanSansSC-Bold.ttf"
font_end:
font_size:
    .int font_end - font

style:
    .incbin "style.css"
style_end:
style_size:
    .int style_end - style

tw2s:
    .incbin "/usr/local/share/opencc/tw2s.json"
tw2s_end:
tw2s_size:
    .int tw2s_end - tw2s

TSPhrases:
    .incbin "/usr/local/share/opencc/TSPhrases.ocd2"
TSPhrases_end:
TSPhrases_size:
    .int TSPhrases_end - TSPhrases

TWVariantsRevPhrases:
    .incbin "/usr/local/share/opencc/TWVariantsRevPhrases.ocd2"
TWVariantsRevPhrases_end:
TWVariantsRevPhrases_size:
    .int TWVariantsRevPhrases_end - TWVariantsRevPhrases

TWVariantsRev:
    .incbin "/usr/local/share/opencc/TWVariantsRev.ocd2"
TWVariantsRev_end:
TWVariantsRev_size:
    .int TWVariantsRev_end - TWVariantsRev

TSCharacters:
    .incbin "/usr/local/share/opencc/TSCharacters.ocd2"
TSCharacters_end:
TSCharacters_size:
    .int TSCharacters_end - TSCharacters

geetest_js:
    .incbin "geetest.js"
geetest_js_end:
geetest_js_size:
    .int geetest_js_end - geetest_js

index_html:
    .incbin "index.html"
index_html_end:
index_html_size:
    .int index_html_end - index_html
