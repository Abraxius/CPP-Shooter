# CPP-Shooter
Ein 3D-Shooter in C++, umgesetzt mit OpenGL und SDL3. Für das Master-Modul "Computer Graphics".

## Wichtig
- Bei SDL3_Mixer gibt es aktuell leider einige Probleme, weshalb die Audios auskommentiert sind. Sie sind mit "ToDo" markiert an den Stellen, an denen sie ansonsten eingefügt wären.
- Beim Spawnen und Löschen von Objekten wie Zombies oder Projektilen kann es dazu kommen, dass das Projekt abstürzt. Das scheint mit dem UI im GameScreen zusammenzuhängen. Aus diesem Grund ist das Game-UI in Zeile 95 in der Datei "App.hpp" auskommentiert. Wenn Sie sich das UI anschauen möchten, müssen Sie es einfach nur wieder einkommentieren. Allerdings kann es dann passieren, dass die Anwendung beim Schießen abstürzt. Das GameScreen-UI besteht aus einem Fadenkreuz, einem ZombieScore und einem PlayerScore.

## Spielsteuerung
- StartScreen: Wie im UI beschrieben, ist es möglich, das Spiel vom StartScreen aus mit der Taste Q zu starten.
- Wichtig: Da es sich bei dem Spiel um einen Zombie-Shooter handelt, beachten Sie, dass die Zombies nur Schaden bekommen, wenn Sie ihnen in den Kopf schießen!
- GameScreen: Hier ist eine übliche Steuerung möglich. Mit W A S D können Sie laufen und mit der Leertaste springen. Mit linker Shift-Taste ist es möglich zu sprinten. Ebenfalls ist es mit der Maus möglich zu zielen und mit der linken Maustaste zu schießen. Wenn die Zombies zu nah an Sie herankommen, fügen sie Ihnen Schaden zu und können Sie töten.
- EndScreen: Hier sehen Sie Ihren HighScore und können das Spiel mit E beenden.

## Code
### Raycast
- Im Ordner "Weapon" gibt es 3 Klassen, die für die Waffen zuständig sind.
- Die "RaycastHit.hpp" ist dabei die wichtigste, da sie einen Raycast von der Mitte des Bildschirms in die Blickrichtung berechnet.
### Enemy
- Im Ordner "Enemy_System" befinden sich 2 Klassen, die für die Enemies zuständig sind.
- Dabei handelt es sich um ein Verwaltungssystem "Enemy_System.hpp" und die Klasse "Enemy.hpp" selbst.
### Player
- Die Attribute des Spielers werden in "Game_Objects/Player.hpp" verwaltet.
- Die Steuerung des Spielers wird in "App.hpp" in der Methode "Handle_Input()" verwaltet. Dort erhält der Spieler unter anderem auch Schwerkraft.

## Modelle
Alle verwendeten Modelle sind lizenzfrei und für den nichtkommerziellen Gebrauch freigegeben.
