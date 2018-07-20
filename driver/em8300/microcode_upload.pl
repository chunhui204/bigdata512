#!/usr/bin/perl

my @devs = ("/dev/em8300","/dev/em8300-1","/dev/em8300-2","/dev/em8300-3");
$_IOC_NRBITS    =  8;
$_IOC_TYPEBITS  =  8;
$_IOC_SIZEBITS  = 14;
$_IOC_DIRBITS   =  2;

$_IOC_NRMASK    = ((1 << $_IOC_NRBITS)-1);
$_IOC_TYPEMASK  = ((1 << $_IOC_TYPEBITS)-1);
$_IOC_SIZEMASK  = ((1 << $_IOC_SIZEBITS)-1);
$_IOC_DIRMASK   = ((1 << $_IOC_DIRBITS)-1);

$_IOC_NRSHIFT    = 0;
$_IOC_TYPESHIFT  = ($_IOC_NRSHIFT+$_IOC_NRBITS);
$_IOC_SIZESHIFT  = ($_IOC_TYPESHIFT+$_IOC_TYPEBITS);
$_IOC_DIRSHIFT   = ($_IOC_SIZESHIFT+$_IOC_SIZEBITS);

sub _IOC {
  local ($dir,$type,$nr,$size) = @_;

  ((($dir)  << $_IOC_DIRSHIFT) | 
   ((ord($type)) << $_IOC_TYPESHIFT) | 
   (($nr)   << $_IOC_NRSHIFT) | 
   (($size) << $_IOC_SIZESHIFT));
}

sub EMCTL_IOCTL_INIT { _IOC(1,'C',0,8 );}
sub EMCTL_IOCTL_READREG { _IOC(3,'C',1,8);}
sub EMCTL_IOCTL_WRITEREG { _IOC(1,'C',2,8); }
sub EMCTL_IOCTL_GETSTATUS { _IOC(2,'C',3,shift)}

# Read microcode file
open (UCODE,"<$ARGV[0]") or die("Can't open microcode file: $ARGV[0]");
undef $/;
$ucode=<UCODE>;
close UCODE;

# Open device
foreach (@devs){
	open (DEV,"<$_") or exit(0);
	
# Prepare ioctl
$initparams = pack("PI", $ucode, length($ucode));

if(!ioctl(DEV, &EMCTL_IOCTL_INIT, $initparams)) {
  print "Microcode upload failed: $!\n";
  exit(1);
}

print "Microcode uploaded to $_\n";

close DEV; 
}
