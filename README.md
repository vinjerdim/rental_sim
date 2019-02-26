# DESKRIPSI SISTEM
Sebuah sistem melayani transportasi bus penumpang antara rental mobil dan terminal
Sistem terdiri atas 3 antrian, yaitu:
-> 2 antrian di terminal (antrian 1 dan 2) 
-> 1 antrian di rental mobil (antrian 3)
Interarrival time setiap antrian terdistribusi secara eksponensial
dengan rerata masing-masing distribusi adalah 14, 10, dan 24

Penumpang dari terminal hendak pergi ke rental mobil
Penumpang dari rental mobil hendak pergi ke salah satu terminal
dengan peluang sebagai berikut:
-> Terminal 1: 0.583
-> Terminal 2: 0.417

Setiap kali bus tiba di salah satu antrian, hal berikut harus terpenuhi:
1. Bus menurunkan (unload) penumpang dahulu. Waktu untuk menurunkan 1 penumpang 
   terdistribusi uniform dalam range 16 - 24 detik
2. Bus kemudian menaikkan (load) penumpang. Waktu untuk menurunkan 1 penumpang 
   terdistribusi uniform dalam range 15 - 25 detik
3. Bus diam di setiap antrian selama minimal 5 menit

