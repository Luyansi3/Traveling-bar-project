import cv2
import os
import tkinter as tk
from PIL import Image, ImageTk
from tkinter import ttk
import time
import socket
from threading import Thread
import numpy as np


source = 1  #indiquer source de la vidéo à afficher
PORT = 5000
IP = "192.168.137.137"




class App :
    def __init__(self, source) :
        self.mainWindow = tk.Tk()
        self.mainWindow.title('essai')
        self.IP = ""
        self.id_affichage = None
        self.id_etat_video = None
        self.pair = 0
        self.fenetre_time_laps = None
        self.etat_connexion = False
        self.choix_scenario = tk.StringVar()
        self.choix_scenario.set("Time laps")
        self.position_moteur_A = 0
        self.position_moteur_B = 0
        self.ancienne_pos_A = 0
        self.ancienne_pos_B =0
        self.compteur_dossier_TL = 0
        self.premier_clic = True
        self.le_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.mainWindow.attributes('-fullscreen', True)
        self.mainWindow.bind("<KeyPress-Escape>", self.quit)
        self.camera = MyCamera(source)
        self.color=100
        self.lo=np.array([self.color-5, 100, 50])
        self.hi=np.array([self.color+5, 255,255])
        self.color_info=(0, 0, 255)
        self.pret = False
        self.reconnaissance_activer = False
        self.t0=0

        
        self.widgets(self.mainWindow)
        self.timer_etat_video()
        
        
        

    def widgets(self, root) :

        self.textIp = tk.Label(root, text = "Entrez IP ESP :")
        self.textIp.place(x = 350, y = 175)

        self.entre_ip = tk.Entry(root, width=40)
        self.entre_ip.place(x = 435, y = 175)
        self.entre_ip.insert(0, "192.168.137.148")

        self.connect = tk.Button(root, text= "Connecter", background = 'yellow')
        self.connect.place(x = 690, y = 170 )
        self.connect.bind('<Button-1>', self.connexion)

        self.etat_connecte = tk.Label(root, text = "Non connecté")
        self.etat_connecte.place(x = 770, y = 175)

        self.bouton_video = tk.Button(root, background = 'yellow', text = 'afficher Vidéo', width = 40)
        self.bouton_video.place(x = 50, y = 130)
        self.bouton_video.bind('<Button-1>', self.affichage_window_camera)

        self.bouton_quitter = tk.Button(root, background = 'yellow', text = 'quitter', width = 40)
        self.bouton_quitter.place(x = 50, y = 170)
        self.bouton_quitter.bind('<Button-1>', self.destroy)

        self.aller_droite = tk.Button(root, background = 'yellow', text = 'Aller à droite', width = 20, command=self.translation_droite, repeatdelay=1, repeatinterval=1000)
        self.aller_droite.place(x = 1360, y = 200)
        self.aller_droite.bind('<ButtonRelease-1>', lambda event: self.ralentir(event, "A"))

        self.aller_gauche = tk.Button(root, background = 'yellow', text = 'Aller à gauche', width = 20, command=self.translation_gauche, repeatdelay=1, repeatinterval=1000)
        self.aller_gauche.place(x = 1200, y = 200)
        self.aller_gauche.bind('<ButtonRelease-1>', lambda event: self.ralentir(event, "A"))

        self.touner_gauche = tk.Button(root, background = 'yellow', text = 'Tourner à gauche', width = 20, command=self.rotation_gauche, repeatdelay=1, repeatinterval=1000)
        self.touner_gauche.place(x = 1200, y = 230)
        self.touner_gauche.bind('<ButtonRelease-1>', lambda event: self.ralentir(event, "B"))


        self.touner_droite = tk.Button(root, background = 'yellow', text = 'Tourner à droite', width = 20, command=self.rotation_droite, repeatdelay=1, repeatinterval=1000)
        self.touner_droite.place(x = 1360, y = 230)
        self.touner_droite.bind('<ButtonRelease-1>', lambda event: self.ralentir(event, "B"))

        self.affichage_message = tk.Label(root, text = "Message recu de l'arduino :")
        self.affichage_message.place(x = 1280, y = 270)

        self.affichage_pos_A = ttk.Progressbar(root, orient = "horizontal", length = 200, mode = 'determinate', value= 0)
        self.affichage_pos_A.place(x = 1300, y = 320)

        self.message_pos_A = tk.Label(root, text = "Position transversale Caméra : ").place(x = 1130, y = 320)

        self.btn_calibration = tk.Button(root, background = 'yellow', text = 'Calibration', width = 20, command = self.calibration)
        self.btn_calibration.place(x = 1280, y = 150 )
        
        self.affichage_vitesse_motor_A = tk.Label(root, text = "Vitesse de la translation :").place(x = 1130, y = 370)
        self.curseur_vitesse_motor_A = tk.Scale(root, from_ = 95, to = 255, length = 200, orient='horizontal')
        self.curseur_vitesse_motor_A.place(x = 1298, y = 350)

        self.affichage_vitesse_motor_B = tk.Label(root, text = "Vitesse de la rotation :").place(x = 1130, y = 415)
        self.curseur_vitesse_motor_B = tk.Scale(root, from_ = 100, to = 200, length = 200, orient='horizontal')
        self.curseur_vitesse_motor_B.place(x = 1298, y = 395)
        

        self.titre_scenario = tk.Label(root, text = "Création scénario")
        self.titre_scenario.place(x = 1280, y = 470)

        self.affichage_choix_temps = tk.Label(root, text= "Choisir durée scénario : ").place(x = 1130, y = 500)
        self.choix_temps = tk.Spinbox(root, from_= 60, to = 900, width = 10)
        self.choix_temps.place(x = 1300, y = 500)

        self.affichage_choix_rotation = tk.Label(root, text= "Choisir rotation scénario : ").place(x = 1130, y = 535)
        self.choix_rotation = tk.Spinbox(root, from_= 1, to = 720, width = 10)
        self.choix_rotation.place(x = 1300, y = 535)

        self.lancer_scenario = tk.Button(root, text= "Lancer le scnénario", bg = 'yellow' , width = 20)
        self.lancer_scenario.place(x = 1280,y= 610)
        self.lancer_scenario.bind('<Button-1>', self.scenario)

        self.time_laps = tk.Radiobutton(root, text="Time laps", variable=self.choix_scenario, value="Time laps")
        self.time_laps.place(x = 1400, y = 500)

        self.scenario_video = tk.Radiobutton(root, text="Vidéo", variable=self.choix_scenario, value="Vidéo")
        self.scenario_video.place(x = 1400, y = 535)

        self.affichage_choix_dt = tk.Label(root, text= "Delta t entre photos: ").place(x = 1130, y = 570)
        self.choix_dt = tk.Spinbox(root, from_= 1, to = 60, width = 10)
        self.choix_dt.place(x = 1300, y = 570)

        self.bouton_photo = tk.Button(root, text = "Prendre une photo", command = lambda: self.camera.prendre_photo("Photos/Photo "+str(self.camera.compteur_photo)+" .jpg"),  bg = 'yellow', width = 20)
        self.bouton_photo.place(x = 250, y = 825)

        self.bouton_photo = tk.Button(root, text = "Prendre une vidéo", command = lambda: self.camera.prendre_video("Vidéos/Vidéo "+str(self.camera.compteur_video)+" .avi"),  bg = 'yellow', width = 20)
        self.bouton_photo.place(x = 450, y = 825)

        self.etat_video = tk.Label(root, text="")
        self.etat_video.place(x = 675, y = 825)


        self.bouton_reconnaissance = tk.Button(root, text = "Activer/Désactiver reconnaisance d'objet", bg = 'yellow')
        self.bouton_reconnaissance.place(x = 1240,y= 700)
        self.bouton_reconnaissance.bind('<Button-1>', self.reconnaissance)






        
    
    def connexion(self, event):
        self.IP = self.entre_ip.get()
        print(self.IP)
        self.le_socket.connect((self.IP, PORT))
        self.etat_connexion = True
        self.etat_connecte.config(text = "Connecté !")

        self.msg_recu = " "
        self.thread_recu = Thread(target=self.reception)
        self.thread_recu.start()




    def reception(self):

        while self.msg_recu != "STOP":
            b_msg = self.le_socket.recv(5000)
            try :
                self.msg_recu = b_msg.decode("utf-8")
            except UnicodeDecodeError:
                self.msg_recu = " "
            if self.fenetre_time_laps != None and "/POSA" in self.msg_recu.split(" "):
                print(self.msg_recu.split(" "))
                try :
                    self.position_moteur_A = int(self.msg_recu.split(" ")[2])
                except ValueError:
                    self.position_moteur_A = 0
                self.bar_time_laps["value"] = (self.position_moteur_A/29) *100

            elif "/POSA" in self.msg_recu.split(" "):
                #print(self.msg_recu.split(" "))
                try :
                    self.position_moteur_A = int(self.msg_recu.split(" ")[2])
                    self.ancienne_pos_A =  self.position_moteur_A
                except ValueError:
                    self.position_moteur_A = self.ancienne_pos_A 
                self.affichage_pos_A["value"] = (self.position_moteur_A/29) *100

            elif "/POSB" in self.msg_recu.split(" "):
                #print(self.msg_recu.split(" "))
                try : 
                    self.position_moteur_B = int(self.msg_recu.split(" ")[2])
                    self.ancienne_pos_B =  self.position_moteur_B
                except ValueError:
                    self.position_moteur_B = self.ancienne_pos_B
                

            else:
                self.affichage_message.config(text = self.msg_recu)
            #time.sleep(0.05)
            self.mainWindow.update()
            #time.sleep(0.05)
             

        #print("fin timer")

    def affichage_window_camera(self, event) :
        self.mainWindow.title("Camera")
        self.mainWindow.canevas = tk.Canvas(self.mainWindow, width = 1100, height = 618)
        self.mainWindow.canevas.place(x= 20, y = 200)
        self.delay = 10
        self.video()

    def video(self) :
        ret, frame = self.camera.frame()
        if ret :
            
            if self.reconnaissance_activer :
                
                self.image_temp=cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
                self.image_temp=cv2.blur(self.image_temp, (5, 5))
                mask=cv2.inRange(self.image_temp, self.lo, self.hi)
                mask=cv2.erode(mask, None, iterations=4)
                mask=cv2.dilate(mask, None, iterations=4)

                elements=cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]

                if len(elements) > 0:
                    c=max(elements, key=cv2.contourArea)
                    ((self.x, self.y), radius)=cv2.minEnclosingCircle(c)
                    self.position_objet_x = self.x
                    self.position_objet_y = self.y
                    if radius>30 and not self.pret:
                        cv2.circle(self.image_temp, (int(self.x), int(self.y)), 5, self.color_info, 10)
                        cv2.line(self.image_temp, (int(self.x), int(self.y)), (int(self.x)+150, int(self.y)), self.color_info, 2)
                        cv2.putText(self.image_temp, "Objet !!!", (int(self.x)+10, int(self.y) -10), cv2.FONT_HERSHEY_DUPLEX, 1, self.color_info, 1, cv2.LINE_AA)
                    cv2.circle(frame, (int(self.x), int(self.y)), 5, self.color_info, 10)
                    cv2.line(frame, (int(self.x), int(self.y)), (int(self.x)+150, int(self.y)), self.color_info, 2)
                    cv2.putText(frame, "Objet !!!", (int(self.x)+10, int(self.y) -10), cv2.FONT_HERSHEY_DUPLEX, 1, self.color_info, 1, cv2.LINE_AA)
                
                if not self.pret:
                    cv2.putText(frame, "Couleur: {:d}".format(self.color), (10, 30), cv2.FONT_HERSHEY_DUPLEX, 1, self.color_info, 1, cv2.LINE_AA)
                    cv2.imshow('Camera', frame)
                    cv2.imshow('Mask', mask)
                else:
                    self.tracking()
                    

                if cv2.waitKey(1)&0xFF==ord('q'):
                    self.pret = True
                    cv2.destroyWindow('Camera')
                    cv2.destroyWindow('Mask')
                    
            tkimage = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            image = Image.fromarray(tkimage)
            image_resize = image.resize((1100, 619))
            self.pic = ImageTk.PhotoImage(image = image_resize) 
            self.mainWindow.canevas.create_image(0, 0, image = self.pic, anchor=tk.NW)
            

        self.id_affichage = self.mainWindow.after(self.delay, self.video)

    def souris(self, event, x, y, flags, param):
        if event==cv2.EVENT_LBUTTONDBLCLK:
            self.color=self.image_temp[y, x][0]
            print(x,y)
        if event==cv2.EVENT_MOUSEWHEEL:
            if flags<0:
                if self.color>5:
                    self.color-=1
            else:
                if self.color<250:
                    self.color+=1
        self.lo[0]=self.color-5
        self.hi[0]=self.color+5

    def reconnaissance(self,event):
        self.pret = False
        if self.reconnaissance_activer :
            self.reconnaissance_activer = False
        else : 
            self.reconnaissance_activer = True
            cv2.namedWindow('Camera')
            cv2.setMouseCallback('Camera', self.souris)

    
    def tracking(self):

        t1 = time.time()
        if t1 - self.t0 > 0.3:
            self.position_objet_x = round(self.position_objet_x, 0)
            print(self.position_objet_x, self.position_objet_y)
            #max gauche 500 milieu 650 max droite 800
            """if self.position_objet_x <440 :
                if self.position_moteur_A >= 3:
                    print(f"/MEGA /TR /A {self.position_moteur_A - 5};\n")
                    msg= f"/MEGA /TR /A {self.position_moteur_A - 5};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)
                else:
                    print(f"/MEGA /TR /B {self.position_moteur_B +10};\n")
                    msg= f"/MEGA /TR /B {self.position_moteur_B +10};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)

            elif self.position_objet_x > 830:
                if self.position_moteur_A <= 26:
                    print(f"/MEGA /TR /A {self.position_moteur_A + 5};\n")
                    msg= f"/MEGA /TR /A {self.position_moteur_A + 5};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)
                else:
                    print(f"/MEGA /TR /B {self.position_moteur_B-10};\n")
                    msg= f"/MEGA /TR /B {self.position_moteur_B-10};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)"""
            if self.position_moteur_A < 28 and self.position_moteur_A >=1:
                print(f"/MEGA /TR /A {self.position_objet_x-650};\n")
                self.le_socket.sendall(bytes(f"/MEGA /TR /A {self.position_objet_x-650};\n", "utf-8"))

            else : 
                print(f"/MEGA /TR /B {self.position_objet_x-650};\n")
                self.le_socket.sendall(bytes(f"/MEGA /TR /B {self.position_objet_x-650};\n", "utf-8"))

            self.t0 =t1
    
    def destroy(self, event) :
        if self.etat_connexion:
            msg= "STOP\n"
            msg = bytes(msg, "utf-8")
            self.le_socket.sendall(msg)
            time.sleep(0.5)
        
            self.le_socket.close()
            self.etat_connexion = False
            self.etat_connecte.config(text = "Non connecté")
        self.le_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if self.id_affichage != None:
            self.mainWindow.after_cancel(self.id_affichage)
        if self.id_etat_video != None:
            self.mainWindow.after_cancel(self.id_etat_video)
        cv2.destroyAllWindows()
        self.mainWindow.canevas.destroy()

    def quit(self, event):
        if self.etat_connexion:
            msg= "STOP\n"
            msg = bytes(msg, "utf-8")
            self.le_socket.sendall(msg)
            time.sleep(0.5)

            self.mainWindow.update()
            self.le_socket.close()
            self.etat_connexion = False
        
        self.stop = True
        cv2.destroyAllWindows()        
        self.mainWindow.destroy()





    def translation_gauche(self):
        self.vitesse_motor_A = self.curseur_vitesse_motor_A.get()
        self.bouton_press = "gauche"
        if self.premier_clic:
            for i in range(self.vitesse_motor_A-40, self.vitesse_motor_A,20):
                if self.position_moteur_A < 29-(0.01*self.vitesse_motor_A):
                    msg= f"/MEGA /MOTORA {i};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)
                    #print(f"/MEGA /MOTORA {i};")
                    time.sleep(0.05)

            self.premier_clic = False

        if self.position_moteur_A < 29-(0.01*self.vitesse_motor_A):
            msg= f"/MEGA /MOTORA {self.vitesse_motor_A};\n"
            msg = bytes(msg, "utf-8")
            self.le_socket.sendall(msg)
            #print(f"/MEGA /MOTORA {self.vitesse_motor_A};")


    def translation_droite(self):
        self.vitesse_motor_A = self.curseur_vitesse_motor_A.get()
        self.bouton_press = "droite"
        #print("droite")
        if self.premier_clic:
            for i in range(self.vitesse_motor_A-40, self.vitesse_motor_A,20):
                if self.position_moteur_A > 0.01*self.vitesse_motor_A:
                    msg= f"/MEGA /MOTORA {-i};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)
                    print(f"/MEGA /MOTORA {-i};")
                    time.sleep(0.05)

            self.premier_clic = False

        if self.position_moteur_A > 0.01*self.vitesse_motor_A:
            msg= f"/MEGA /MOTORA {-self.vitesse_motor_A};\n"
            msg = bytes(msg, "utf-8")
            self.le_socket.sendall(msg)
            print(f"/MEGA /MOTORA {-self.vitesse_motor_A};")

    def rotation_gauche(self):
        self.vitesse_motor_B = self.curseur_vitesse_motor_B.get()
        self.bouton_press = "gauche"
        if self.premier_clic:
            for i in range(self.vitesse_motor_B-20, self.vitesse_motor_B,10):
                msg= f"/MEGA /MOTORB {i};\n"
                msg = bytes(msg, "utf-8")
                self.le_socket.sendall(msg)
                print(f"/MEGA /MOTORB {i};")
                time.sleep(0.05)

            self.premier_clic = False

        msg= f"/MEGA /MOTORB {self.vitesse_motor_B};\n"
        msg = bytes(msg, "utf-8")
        self.le_socket.sendall(msg)
        print(f"/MEGA /MOTORB {self.vitesse_motor_B};")


    def rotation_droite(self):
        self.vitesse_motor_B = self.curseur_vitesse_motor_B.get()
        self.bouton_press = "droite"
        if self.premier_clic:
            for i in range(self.vitesse_motor_B-20, self.vitesse_motor_B,10):
                msg= f"/MEGA /MOTORB {-i};\n"
                msg = bytes(msg, "utf-8")
                self.le_socket.sendall(msg)
                print(f"/MEGA /MOTORB {-i};")
                time.sleep(0.05)

            self.premier_clic = False

        msg= f"/MEGA /MOTORB {-self.vitesse_motor_B};\n"
        msg = bytes(msg, "utf-8")
        self.le_socket.sendall(msg)
        print(f"/MEGA /MOTORB {-self.vitesse_motor_B};")

    def calibration(self):
        msg= "/MEGA Calibration;\n"
        msg = bytes(msg, "utf-8")
        self.position_moteur_A = 0
        self.le_socket.sendall(msg)


    def ralentir(self, event, moteur):
        if moteur == "A":
            for i in range(self.vitesse_motor_A, self.vitesse_motor_A-60,-20):
                if self.position_moteur_A < 29-(0.01*self.vitesse_motor_A):
                    if self.bouton_press == "gauche":
                        msg= f"/MEGA /MOTORA {i};\n"
                        msg = bytes(msg, "utf-8")
                        self.le_socket.sendall(msg)
                        #print(f"/MEGA /MOTORA {i};")
                        time.sleep(0.05)
                    elif self.bouton_press == "droite":
                        msg= f"/MEGA /MOTORA {-i};\n"
                        msg = bytes(msg, "utf-8")
                        self.le_socket.sendall(msg)
                        #print(f"/MEGA /MOTORA {-i};")
                        time.sleep(0.05)
            
            msg= f"/MEGA /MOTORA {0};\n"
            msg = bytes(msg, "utf-8")
            self.le_socket.sendall(msg)
            #print(f"/MEGA /MOTORA {0};")

        elif moteur == "B":
            for i in range(self.vitesse_motor_B, self.vitesse_motor_B-20,-10):
                if self.bouton_press == "gauche":
                    msg= f"/MEGA /MOTORB {i};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)
                    print(f"/MEGA /MOTORB {i};")
                    time.sleep(0.05)
                elif self.bouton_press == "droite":
                    msg= f"/MEGA /MOTORB {-i};\n"
                    msg = bytes(msg, "utf-8")
                    self.le_socket.sendall(msg)
                    print(f"/MEGA /MOTORB {-i};")
                    time.sleep(0.05)

            msg= f"/MEGA /MOTORB {0};\n"
            msg = bytes(msg, "utf-8")
            self.le_socket.sendall(msg)
            print(f"/MEGA /MOTORB {0};")

        self.premier_clic = True

    def scenario(self, event):
        self.calibration()
        if self.choix_scenario.get() == "Time laps":
            self.lancer_time_laps()
        else:
            self.lancer_scenario_video()

    def lancer_time_laps(self):
        self.fenetre_time_laps = tk.Toplevel(self.mainWindow)
        self.fenetre_time_laps.title("Fenêtre scénario time laps")
        self.fenetre_time_laps.geometry("300x200")
        self.stop = False
        self.compteur_photos_TL = 0
        self.compteur_dossier_TL +=1
        if not os.path.exists("Time Laps "+str(self.compteur_dossier_TL)):
            os.mkdir("Time Laps "+str(self.compteur_dossier_TL))
        
        self.fenetre_time_laps.grab_set()

        self.bar_time_laps = ttk.Progressbar(self.fenetre_time_laps, orient = "horizontal", length = 200, mode = 'determinate', value= 0)
        self.bar_time_laps.pack()

        self.bouton_stop_time_laps = tk.Button(self.fenetre_time_laps, text = "ARRET DU TIMELAPS")
        self.bouton_stop_time_laps.pack()
        self.bouton_stop_time_laps.bind('<Button-1>', self.arret_time_laps)

        self.temps_time_laps = float(self.choix_temps.get())
        self.rotation_time_laps = int(self.choix_rotation.get())

        self.mainWindow.update()

        dt_photos = float(self.choix_dt.get())
        nb_photos = int(self.temps_time_laps/dt_photos)
        dist_entre_photo = 29/nb_photos
        dist_entre_photo = round(dist_entre_photo, 2)

        rota_entre_photo = round(self.rotation_time_laps/nb_photos, 2)

        ancienne_pos_A = 0
        ancienne_pos_B = 0

        for i in range(nb_photos+1):
            if not self.stop: 
                print(f"/MEGA /TL {ancienne_pos_A}          {ancienne_pos_B};\n")
                self.le_socket.sendall(bytes(f"/MEGA /TL {ancienne_pos_A}          {ancienne_pos_B};\n", "utf-8"))
                ancienne_pos_B = round(ancienne_pos_B +rota_entre_photo, 2)
                ancienne_pos_A = round(ancienne_pos_A+dist_entre_photo,2)
                self.mainWindow.update()
                time.sleep(2*dt_photos/3)
                self.camera.prendre_photo("Time Laps "+str(self.compteur_dossier_TL)+ "/Photo "+str(self.compteur_photos_TL)+" .jpg")
                self.compteur_photos_TL += 1
                time.sleep(dt_photos/3)
            else:
                break
        if not self.stop :
            self.arret_time_laps(1)

    def lancer_scenario_video(self):
        pass

    def arret_time_laps(self, event):
        self.stop = True
        self.fenetre_time_laps.grab_release()
        self.fenetre_time_laps.destroy()
        self.fenetre_time_laps = None
        self.mainWindow.update()


    def timer_etat_video(self):
        if self.camera.enregistre and self.pair == 0:
            self.pair = 1
            self.etat_video.config(text = "Enregistre en cours !", fg = "#000000")
            self.id_etat_video = self.mainWindow.after(1000, self.timer_etat_video)
        elif self.camera.enregistre and self.pair == 1:
            self.pair = 0
            self.etat_video.config(text = "Enregistre en cours !", fg = "#ff0000")
            self.id_etat_video = self.mainWindow.after(1000, self.timer_etat_video)
        else:
            self.etat_video.config(text = "", fg = "#ff0000")
            self.id_etat_video = self.mainWindow.after(1000, self.timer_etat_video)

class MyCamera :
    def __init__(self, source) :
        self.source = source
        self.connect()
        self.compteur_photo = 0
        self.compteur_video = 0
        self.enregistre = False
        self.width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        self.fps = self.cap.get(cv2.CAP_PROP_FPS)
        self.codec = cv2.VideoWriter_fourcc(*'XVID')
        
    def connect(self) : 
        self.cap = cv2.VideoCapture(self.source)
        if not self.cap.isOpened():
            print(f"Impossible d'ouvrir la camera de la source {self.source}")
    
    def frame(self) :
        if self.cap.isOpened(): 
            ret, frame = self.cap.read()
            if ret :
                if self.enregistre:
                    self.video_writer.write(frame)
                return (ret, frame)
            else : 
                return (ret, None)
        else :
            self.connect()
    
    def destroy_cam(self) :
        if self.cap.isOpened() :
            self.cap.release()

    def prendre_photo(self, nom_photo):
        ret, frame = self.cap.read()
        if ret:
            cv2.imwrite(nom_photo, frame)
            self.compteur_photo += 1

    def prendre_video(self, nom_video):
        if self.enregistre:
            self.video_writer.release()
            self.enregistre = False
        else:
            self.video_writer = cv2.VideoWriter(nom_video, self.codec, self.fps, (self.width, self.height))
            self.enregistre = True
            self.compteur_video +=1

if __name__ == '__main__' : 
    app = App(source) # création appli
    app.mainWindow.mainloop() 