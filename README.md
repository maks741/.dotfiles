### Issues and solutions

* Pacman slow download speed  
  Update your mirrors in /etc/pacman.d/mirrorlist
  1. Create a backup of current mirrorlist  
    cp /etc/pacman.d/mirrorlist /etc/pacman.d/backup_mirrorlist
  2. Use reflector to update the mirror list  
    reflector --verbose --country <your_country> --age 12 --protocol https --sort rate --save /etc/pacman.d/mirrorlist
  3. Sync mirrors  
    pacman -Syyu  

  Useful links:
  1. https://www.reddit.com/r/archlinux/comments/13shhq2/very_slow_download_speed_in_pacman/
  2. https://wiki.archlinux.org/title/Mirrors
  3. https://wiki.archlinux.org/title/Reflector
  4. https://man.archlinux.org/man/reflector.1#EXAMPLES
  5. https://bbs.archlinux.org/viewtopic.php?id=257047
