Viktiga Anmärkningar:

    QtMultimedia: Se till att du har modulen QtMultimedia installerad och att du lägger till QT += multimedia i din .pro-fil.

    Sökväg: Ändra variabeln wavFolder i ditt main.cpp till den korrekta sökvägen där dina .wav-filer finns.

    Filnamn: Funktionen playWav() tar endast filnamnet, utan sökvägen och utan .wav-ändelsen (t.ex. om filen heter alarm.wav anropar du manager.playWav("alarm")).

    QSoundEffect vs. QMediaPlayer: QSoundEffect är bäst för korta, okomprimerade ljudeffekter (som varningsljud eller klick). Om du behöver spela längre ljudfiler, strömmande innehåll, eller använda komprimerade format som MP3, bör du använda QMediaPlayer istället.
