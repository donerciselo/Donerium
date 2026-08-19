# Donerium

GTA 5: Enhanced için geliştirilmiş, Türkçe arayüzlü mod menüsü.

YimMenuV2 tabanlıdır; tamamen yerelleştirilmiş arayüz, özel mavi/cam tema ve aşağıdaki ekstra özelliklerle birlikte gelir.

## Öne çıkan özellikler

- **Oto Pilot** — Haritada işaretlediğin noktaya otomatik sürüş
  - Hız Modu: Eco (100 km/s) / Comfort (150 km/s) / Sport (son hız)
  - Sürüş Stili: Normal / Işıkları Geç / Agresif (agresif modda sollamada sinyal yakar)
  - Hava Modu (yağmurda/karda yavaşla) | Gece Modu (farları aç) | Polis Kaçış (tam gaz + lastik/motor koruması)
  - Ekranda canlı HUD: hız modu, stil, mesafe, aktif çevre modları
- **Oto Park** — Aracı en yakın yol düğümüne yavaşça yanaştırır
- **Mavi/cam arayüz** — özel tema, DONERIUM markalı başlık ve alt bilgi

## Kurulum

1. FSL'yi [buradan](https://www.unknowncheats.me/forum/grand-theft-auto-v/616977-fsl-local-gtao-saves.html) indir ve `WINMM.dll`'i GTA V klasörüne koy (hesap güvenliği için önerilir)
2. Enjekterör indir: [Xenos](https://www.unknowncheats.me/forum/general-programming-and-reversing/124013-xenos-injector-v2-3-2-a.html)
3. Rockstar Launcher'da GTA V Enhanced'ı seç, Ayarlar'dan BattlEye'i kapat. Steam/Epic kullanıyorsan `-nobattleye` komut satırı parametresini ekle
4. Oyunu başlat ve menüdeyken `Donerium.dll`'i enjekte et

## Menüyü açma

`INSERT` veya `Ctrl+\` tuşlarına bas.

## Sık karşılaşılan sorunlar

### Her beş dakikada bir herkese açık oturumdan atılıyorum

Şu an BattlEye bypass'ımız yok; oturum sahipleri kalp atışı başarısızlığı nedeniyle seni atacaktır. Bunu yalnızca özel bir bypass durdurur.

### FSL'i sildim ve bütün ilerlemem kayboldu!

FSL hesap kayıt verilerini diske yönlendirir; FSL ile yapılan ilerleme yalnızca FSL açıkken görünür. İstiyorsan Donerium'u FSL olmadan da kullanabilirsin, ancak önerilmez.

### FSL'i sildim ve oyun artık açılmıyor!

Bilinen bir sorun: `Documents/GTAV Enhanced/Profiles` klasörünü silecek şekilde düzelt.

## Derleme

Proje CMake kullanır; `cmake` ile kur ve YimMenuV2 ile aynı bağımlılıklarla derle.

> Bu proje GTA V için bir mod menüsüdür. Hile amaçlı kullanabilirsiniz; hesap yasağı riski size aittir.