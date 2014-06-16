# Copyright (c) 2006-2012 Intel Corporation. All rights reserved.

# Validates data in ini files. note it attempts to validate file names
# by regex because there is no other way to validate non-existent ones.

BEGIN {
    allokflag = 1;

    # try this out to see if you can break it
    # I used a regex because the user may not be running as root. in any
    # case we would not want to allow some paths even if they can be
    # created.
    filepat = "^/[a-zA-Z0-9._/-]+$";
    # we don't use --posix for compotibility reasons
    snpat = "^[a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9]-[a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9][a-zA-Z0-9]$" ;
    lspat = "^[0-9]+@[a-zA-Z0-9._-]+$";
    comppat = "^.*$";

    # it would not be very hard to read the data from a file rather than
    # hardwire it as here.

    # this might be useful, if not remove it.

    # exclude table
    exclude["NO_VALIDATE"] = 1;  # example

    # validation data

    # section data is made up of "name1:val1:val2..." blocks separated
    # by ";". value strings are evaluated as regex, and may be fancier
    # than what is shown, for example [Yy][Ee][Ss]. regex can allow
    # arbitrary values, for example TEXT might be "[A-Za-z][A-Za-z_0-9]+"

    sections["silent"] = sections["silent"] "ACCEPT_EULA:^accept$:^decline$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "ACTIVATION:^exist_lic$:^serial_number$:^trial_lic$:^license_file$:^license_server$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "PSET_SERIAL_NUMBER:^$:"snpat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "PSET_LICENSE_FILE:^$:"filepat":"lspat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "INSTALL_MODE:^RPM$:^NONRPM$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "CONTINUE_WITH_OPTIONAL_ERROR:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "CONTINUE_WITH_INSTALLDIR_OVERWRITE:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "PSET_INSTALL_DIR:^$:"filepat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "SEND_USAGE_DATA:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";


    sections["silent"] = sections["silent"] "DRIVER_ACCESS_GROUP:^$:^.*$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "DRIVER_INSTALL_TYPE:^auto$:^prebuilt:^build:^filesonly$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "DRIVER_LOAD:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "DRIVER_BOOT_SCRIPT:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "DRIVER_PER_USER_MODE:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "DRIVER_C_COMPILER:^auto$:"filepat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "DRIVER_KERNEL_SOURCE_DIR:^auto$:"filepat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "DRIVER_MAKE_COMMAND:^auto$:"filepat;
    sections["silent"] = sections["silent"] ";";


    sections["silent"] = sections["silent"] "COMPONENTS:^$:"comppat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "TEMP_FOLDER:^$:"filepat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "AUTOMOUNTED_CLUSTER:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "MACHINES_FILE:^$:"filepat;
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "REMOTE:^yes$:^no$";
    sections["silent"] = sections["silent"] ";";
    sections["silent"] = sections["silent"] "INTEL_LICENSE_FILE:^$:"comppat;
    sections["silent"] = sections["silent"] ";";
}

/^[A-Za-z][-A-Za-z_0-9]*=.*/ {     # name=value pair
    if($0 ~ /..*[ \t].*/)  # check for space
    {
        if(split($0,nameval,"=") == 2)
        {
            name  = nameval[1];
	    value = nameval[2];
            if(!validate(name,value))
            {
	        allokflag = 0;
    	    }		
	}
    }
    else
    {
        if(split($1,nameval,"=") == 2)
        {
            name  = nameval[1];
            value = nameval[2];

            if(!validate(name,value))
            {
                allokflag = 0;
            }
        }
    }

    next;
}

END {
    if(!allokflag)
    {
        print FILENAME " has errors";
	exit 1
    }
    else
    {
	exit 0
    }
}

function validate(name,value)
{
    if(name in exclude)  # skip it
    {
        return 1;
    }

    namefound = 0;

        sectiondata = sections["silent"];

        if((cnt = split(sectiondata,arr,";")))
        {
            for(i=1;i<=cnt;++i)
            {
                # split the name and possibly multiple values
                if((num = split(arr[i],list,":")))
                {
                    # the first one is the name
                    if(list[1] == name)
                    {
                        namefound = 1;
                        # the rest are values
                        for(j=2;j<=num;++j)
                        {
                            # stop looking if we match
		    	    if(value ~ list[j])
                    	    {
                                return 1;
                    	    }
                        }
                    }
                }
            }
        }

    if(namefound)
    {
	if ( name == "ACTIVATION" && value == "remote" )	
	{
	    print "Remote activation is not supported in silent mode. Please use other activation options.";
	}
	else
	{
    	    print "\"" value "\" is not a valid value for " name;
	}
    }
    else
    {
        printf("Name \"%s\" is not valid\n",name);
    }

    return 0;
}
