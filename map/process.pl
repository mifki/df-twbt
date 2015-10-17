#!/usr/bin/perl

use POSIX;

my ($outdir) = @ARGV;

$basex = 0;
$basey = 0;

$regw = 17*16;
$regh = 17*16;

####

$worldw = 17 * 16 * 3;
$worldh = 17 * 16 * 3;

$blockw = $regw * 3;
$blockh = $regh * 3;

#each block img is 256x256 - this is the last level

$maxl = ceil(log($worldw)/log(2));

for (my $l = 0; $l <= $maxl; $l++)
{
	`mkdir -p $outdir/$l`;
}

$l = $maxl;
$bx = $basex*3;
$by = $basey*3;
$w = $blockw;
$h = $blockh;

while ($l > 0)
{
	$l--;
	$w = ceil($w / 2);
	$h = ceil($h / 2);
	$bx = floor($bx / 2);
	$by = floor($by / 2);

	print ("Generating level $l...\n");

	for (my $j = $by; $j < $by+$h; $j++)
	{
		$mk = 0;

		for (my $i = $bx; $i < $bx+$w; $i++)
		{
			$nl = $l + 1;

			$k = 4;
			$f1 = "$outdir/$nl/@{[$j*2+0]}/map_@{[$j*2+0]}_@{[$i*2+0]}.jpg";
			$f2 = "$outdir/$nl/@{[$j*2+0]}/map_@{[$j*2+0]}_@{[$i*2+1]}.jpg";
			$f3 = "$outdir/$nl/@{[$j*2+1]}/map_@{[$j*2+1]}_@{[$i*2+0]}.jpg";
			$f4 = "$outdir/$nl/@{[$j*2+1]}/map_@{[$j*2+1]}_@{[$i*2+1]}.jpg";
			if (! -f $f1) { $f1 = 'empty.png'; $k--; }
			if (! -f $f2) { $f2 = 0&&$i == $bx+$w-1 ? 'empty-bg.png' : 'empty.png'; $k--; }
			if (! -f $f3) { $f3 = 0&&$j == $by+$h-1 ? 'empty-bg.png' : 'empty.png'; $k--; }
			if (! -f $f4) { $f4 = 0&&($i == $bx+$w-1 || $j == $by+$h-1) ? 'empty-bg.png' : 'empty.png'; $k--; }

			if ($k > 0)
			{
				if (!$mk) {
					`mkdir $outdir/$l/$j 2>/dev/null`;
					$mk = 1;
				}

				`montage $f1 $f2 $f3 $f4 -geometry 128x128+0+0 $outdir/$l/$j/map_${j}_${i}.jpg`
				if ($nl == $maxl)
				{
					unlink $f1, $f2, $f3, $f4;
				}
			}
		}

		print ("@{[($j-$by+1)/$h*100]}% done\n");
	}
}
