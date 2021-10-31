import Lab2
import threading

# Gère et envoie les entrées clavier
def keyboard_manager():
    while True:
        char = str(input()).lower()
        if len(char) != 0:
            Lab2.sendInput(char[0])

            if char == "q":
                break


# Initialise les threads du clavier et de la vidéo
def main():
    print(
        "Entrer dans la console le path de la vidéo ou appuyer sur ENTER pour la vidéo par défaut"
    )
    path = str(input())
    print(
        "Entrer dans la console les touches du clavier.\nP - Play ou pause la vidéo.\nA - Accélère la vidéo x1 -> x1.5 -> x2 puis reset à x1.\nR - Retourne au début de la vidéo.\nQ - Quitte le programme."
    )
    threadKeyboard = threading.Thread(target=keyboard_manager)
    threadVideo = threading.Thread(
        target=Lab2.initPlayer, args=["..\\Example.avi" if len(path) == 0 else path]
    )

    threadKeyboard.start()
    threadVideo.start()

    threadKeyboard.join()
    threadVideo.join()


if __name__ == "__main__":
    main()
