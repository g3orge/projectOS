Λειτουργικά Συστήματα Ι - 2012    
Άσκηση 2

----------
Γεώργιος Παπανικολάου - Α.Μ.: 5044    
Προκόπης Γρύλλος - Α.Μ.: 4975

----------
Build for GNU/Linux (x86_64-pc-linux-gnu) with:  
	* gcc 4.6.3
	* make 3.81
	* bash 4.2.24
	* git 1.7.9.5
	* vim 7.3.429

Project hosted on http://github.com/Nacho-Libre/projectOS/tree/threads    
(threads branch)
There is ABSOLUTELY NO WARRANTY

----------

Στην άσκηση αυτή χρησιμοποιύμε δίεργασιές (fork) για να μπορόυν οι διαφορετικες διαδικασίες (π.χ. ψησιμο της πιτσας) να γίνονται παράλληλα. Επίσης σήματα για την εποικοινωνία των διεργασιών, κοινή μνήμη μεταξυ τους για σωστή συνεργασία και sockets για την επικοινωνία client-server, και σημαφόροι για συγχρονισμό.

Ειδικότερα:
Στο pizza.h δηλώνονται οι σταθερες, bool type με enumeration, και το struct της παραγγελιας που περιεχει αριθμο των διαφορετικων ειδών πίτσας της παραγγελιας, μεταβλητες χρόνου, και μία βοηθητικη μεταβλητη exists για την ευκολη διαγραφή. Έδω υπάρχει η δυνατότητα αλλαγης των default μεγεθών της άσκησης. (π.χ.: το χρόνο ψησίματος της μαργαρίτας)

Το client.c υλοποιέι τον πελάτη του συστήματος. Χρημιμοποιει ιδιες δομές με τον server και sockets για να στελνει την παραγγελία. Χωρις ορισματα μπαίνει σε interactive mode για είσοδο παραγγελίας απο το χρηστη, με όρισμα rand παράγει *μια* τυχαία παραγγελία. (Χρηση του run_tests.sh για περισσότερες)

Στο pizza_server.c υλοποιείται ο server.
Ο server δουλέυει σε deamon mode για να είναι στο παρασκήνιο.
Αρχικοποιεί σημαφόρους, κοινη μνήμη, handlers και ανοίγει UNIX socket (/tmp file) στο οποιο περιμένει παραγγελίες. Κάθε φορα που δέχεται μια καινούργια παραγγελία εκτελει fork και αναμένει για άλλες. Το παιδί του εινάι υπευθυνο για την παραγγελία ως σύνολο.(εδώ γίνεται attach στην κοινη μνήμη με shmat) Εκτελεί και αυτο fork για να γίνει το ψήσιμο των πιτσων της παραγγελίας παραλληλα και μετα παραδίδει την παραγγελία.

Θέμα semaphores:
Δημιουργουνταί με sem_open (ένας για τους ψήστες, ενας για τους διανομείς, και ενα mutex για την αποφυγη overwrite/conflict στην κοινή μνήμη) και αρχικοποιούνται κατάλληλα με sem_init (οι δύο πρώτοι στον αριθμό τους και ο τρίτος σε 1 (δυαδικός))
Χρησιμοποιούνται δηλαδη (οι δυο πρώτοι) σαν μετρητές των διαθέσιμων ψηστων/διανομέων. μείωση με sem_wait και αύξηση με sem_post.

Θέμα Coca-coles:
Υλοποιήθηκε σε μία διαφορετική αυτόνομη διεργασία (για αύτο ο αριθμος των σωστων διεργασιων σε stand-by ειναι 2) για να μην επειρεάζει την αναμονη των παραγγελιών. (Σε περίπτωση χρήσης alarm και handler του SIGALRM, η αναμονή των παραγγελίων θα διακόπτωνταν οσο γίνονται οι έλεγχοι, πραγμα ανεπιθύμιτο. Παρ' ολα αυτα υπαρχεί αυτη η διαφορετικη υλοποιηση στο coke-with-handler branch στη σελιδα της ασκησης στο [1]) O κώδικας για το θέμα βρίσκεται στο τέλος του αρχέιου στο label cocacola.

Θέμα sockets:
Τα sockets τα κλείνουμε κατευθείαν μόλις έρθει η παραγγελία για ευκολία και ενημερώνουμε τον  client (συμβολικά) στο logfile, στο οποίο εκτυπώνονται πολλων ειδων μηνυματα απο τον server. Η αναγνώριση της παραγγελίας γίνεται με το αντίστοιχο pid του process.

Το script run_tests.sh χρησιμοποιείται για την εκίνηση πολλών client μαζί. Χωρις ορίσματα στέλνει 100 clients, εναν καθε 0.01 (για να αλλαζουν τα νούμετα της γεννήτριας ψευγοτυχαίων αριθμών). Δυο πιθανά ορίσματα: Πρωτο ο αριθμός των πόσων clients να στέιλει και δέυτερο πιθανο άλλο όνομα του εκτελέσιμου.

----------

Κάποια πράγματα δεν είναι απαραίτητα για τη σωστή λειτουργεία του προγραμματος αλλα ειτε απαιτούνταν στην εκφωνηση ειτε βοηθούσαν στο debugging, οπότε παρέμειναν. (π.χ.: τα statuses των παραγγελιών και οι μεταβλητες χρόνου στις παραγγελιές)  

----------

Γνωστά Προβλήματα:
*Μερικές φορές (μετα απο πολλες παραγγελίες) μενουν <defunct> (zombie) διεργασίες στο process table του λειτουργικου συστήματος παρ' ολα τα μέτρα εναντίον του γεγονότος. (βλ. handling του σήματος SIGCHLD και wait εκέι για απορόφηφη του exit status)

----------
[1]: http://github.com/g3orge/projectOS/tree/coke-with-handler
