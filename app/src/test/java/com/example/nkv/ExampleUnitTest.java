package com.example.nkv;

import org.junit.Assert.*;
import org.junit.Test;

/**
 * Example local unit test, which will execute on the development machine (host).
 * <p>
 * See [testing documentation](http://d.android.com/tools/testing).
 */
public class ExampleUnitTest {

	@Test
	public void foo() {
		String s = "QY\fi��2��A�Ҳ\u0017\u000E\u000F����,@�C\n" +
				"J\u0017�ΈW�T���\u0011\u0019���y�k#e���8\u0010���\u001A�\u0017�.Y��ꤵ��0�X)����d�\u0013�ظ�K>��}�Q�R�z�SU�m\u0000Jc��:6_r�J3:���r�v�L�����\u0010K]q>nA_�,9WkDQ^�krȢ]\u007F\u0001m�٠k!;\u0000�W�8�6�\u001B�*8�5 ������Pף\f�q 2�\u0005p��t�\u0000q�w\u001By�Ϫ�LM_9���JBő�f:\f2eNsd��T�i�:\u0018��;���e�\f�*e�s��eV���}G��!Jw�U���qY{ިeu�m�pۜj)��^���\u0019\u0011;��l���\u0006\u000F���\n" +
				"�>�o��\u007F`�! �w����(\n" +
				"|\u0007��w\u000E��I|8��[\u0002\u001B����A�p\u0016���&�^\u001E�:��\u000FtTvFH�� �(��:%D���\u0015�G_\u0006+~d�<��+��u���;6�\u001Eh���\u0019�bS����R;�o\u001DG;��@]t�\u0015��9vꁩ:���>!=��\u0012\u0012�Cq���H�w���G�\u0007>�ԫ��r�\u001A�\u000Ey���9���>h�j&rLhjp��'\u0001u\u000E_/�n����Jk��\u001B\"8*ׁ�9\u0002���\u0015���8\u000EI &,�n}\u0010$\u000E�j\u000Ept�с����'��E2����rޯj����+�'V�ɋ���[����9�t�(��d\u0010��9�����lw\u001F�1\u0011L��\u0000\tWL�H?��?x@{�67��=EE�ˣ\u0012�f�~ۂu�bz\u0002�:��ߡ>�RU�f�m\u0004��ڔeh\u0006�B˾�T�<Hzɫ�+�\u0011A��~\u0019l�cד]�Q�>��F���[Y��z\n" +
				"�Շi�3cF�� c�9ipS�:V��쀣�+\u0017S\u0010\u0000���7_,�gُ\u001Bj��j�\f�^WQ���\u0000S+��B7̚\u0019x���×�\\!xY$�\u0004��\u0012�z��J�O\u001E�˯ \u007Fұ��<�w{>���j�v\u0014tp�e�\u0007�\u0016`�����\u0005xgR�W�YV�~�|ё��\u0017�\"��P\u001E>�\n" +
				"����(�P#Ϫ\u0018�$`r}\u0017 ۍ-�B\u007F�\u001EZ�����۱gu���\u0013x�nSl�*Ҁ\n" +
				"�y���\u0004\u0010\u0018��\u0000�b���\u0006\u0019���\u000F!�\" G\u000E\u0017;_��T���̟V~�ƳlN\u0014���x�k\u0012�$�ҩ����6w\u0006��\n" +
				"]2��V�WL}\u001C<�}h������2�D���%����}�y�\u000E\u001Dz�P��\n" +
				"\u0003\u0011�\u0003�d�������\u0017S�@\u0019\u0018{��\u007F���'�$�Woe���==o6�1J{7�g\n" +
				"P��|\n" +
				"\u0001X\u00001��\u001D\u0007ä\tY���f���WT&\u00021k\u0012�\u001C2�\u0002��vl�T��*�\n" +
				"R\u000Ft\t2�4B�9�\u0000Z~W\u0001�C��������/G�n\u0018�H�-���\u000E\u0002w)�]�'� P&�\u0001F\u000F�3�\n" +
				"\u0013���$�`\u001EH.�aPSs+Py��ZY\u0019��S��D��\u000B\u0013A\u0018����a�\u00131>�j,�\u001Ddz\u0012ћ�\u000F}[9��\u0018�~J-z�#�Q\u0012����ϴK�\u0002B�сqʣV\u0012�P��\u001F��Ғ\u0005�ֵ]O2*�k���?\u001F�Ě�uߋ7\u0003/���6.��ٹ)Al�\u001D�x=[A\u000E(;\u001D\u0016E�'���@�\u001F�)\"���ͫ��>�B�\n" +
				"��ַ���>�P�|\u0015���\\N !E\u0019\u007F\u0014���\u0013�+�\u001C(��\u0011lH;�x�v0�q\u000E��k\u007FS��x\n" +
				"\u00146\u000E��iZ�\u0005���\u007FⰲV��� 1��n\u001E���I�D�;3\u0012�m�\f�\u0003�A��\u0019��Q��O7\u0004�?��c2��¸-\n" +
				"�X\u001D\u000Fy�-\u001F�F���n�\u000B%��>i �s`ؕx�-P&�9\u001D������#�Q\n" +
				"W� \u0000�E�\u000F�OL�p\n" +
				"?j�7aA���sF��ƊЍ��ޕj��&S۫\u0013Bƾёm�8�f�{46wE�}T>��\u0018k7RY�s��)����<9^Z|\u001F���m�\f��c\u0007��z=8�xS�2�>�\u0011PER�Ȍ\u001928�S��;��0\u0007�f~@`��\u0005&��L�.�`��\n" +
				"�F\u001B)�G\u001D\u0018a�i�gy]����\u0007�\u0014���q�Q!�xd\u001C�+�Jc���x\u001D�v�\u000E�gw\u00006ԝsV���\u000E�,Tb%t�\u0004M��i�s�C\u0005H-}���\u0004�8��܀��_\u000EiO\u0013�$�.����5\u0015��~\u000E�W�sb�Q���.T̜\u0012Y�@3/p�t�c\u0007��s��r�\u0019�Q�M���\u001Ae\u001A\u0018sn\u001E�.��\u0015��&\u000Em\u0004��,{ӻ&�V\u0010`\u000B���\u0015�\u0014vb\u0007�39�\u007F����1\u000B�3e1��5�@\u0017~��Zs�j��ˇ�m�_uJ�{\u000F���bWR8Sw?L����yz��\u000B]��\u0010��\u0003��\u0013f1���4p%\tx����C�V\u0005w>�wO\u0015�,�\u001E��<Q�i\u0005S\u0010���7����iM��\u001B\u000E`NY\u0017�w�jó��\u0016�\u0015��\n" +
				"\u0017\u007Fk�GP���x\u0011 ���㺔�\u0006j��+.�_x�8���yd�<�!�X��M],�Q$\u0018;:�ۥz�ӎ>Lf^�\u0013�n\uF7AB�����d��1\u000Bh��V\u0000\\��\u0012Y/^99?�O�Mp��U���[��\u001A\u0010���V���x35��4�`U\u001F\u0014�Q;����\u0018;o\u0002��\u0010>z\u001C�xTw�,�\u0002�{�;t;�\n" +
				"C���Z�5C\u000Fi٩D�\u001F4r�qU�\u001B��4p� }`�Q�p*\u0000wwkӗ��Y��P��K\u0001�]w�S�ܥ\u0003ȗ�[�Pʹi\u001E��j\u008A�(���:�\u000Fat��6��t��H\u0019�\u0019�eg\u0012u�N�~Ä\u000E\u001F�RX�3��M��\t���\u0001e��\u001F�f�]�����t�l���n��\u001E\uDA7A\uDE0F\u001A=�D�Jx>�Ƒ���:\u000B���z��j�q_�\\+4��D\u0017�$���NsȞ\tE�(��w���� �\u0000Lx\n" +
				"`���&��tq��\u0019���\u001A�\u000E�{nf\n" +
				"�S���߅��E\u0011E��A3ǹ4\u0004\u001B$ $\u0004�@�\u001A�;.\n" +
				"\u1A9C�m���4��\u0001�Q��OW��$�U|�(,���T�&\u001F>��P\u0012�Rއ���l`|��\u0007E\u0013�'���E*\u0012K���B\u0003�\u0016\u001F.ы�\u0379:Kj�\u0012�\u0001��Ų���j0h�\u001D\u000Bv�)�h�\u007FwRl��v9\"\u0015�V�aM���\u0004��<�I��@7���7�(�u�̄ȼЄ\u0003\u000F)X�6�C�7HQ��?i�����*G\u001A]V ��RX�]��\u0000\u001B�U\f��r��ہl�XXCY�\u0011Է#���\u0015��\u0017:\u001ED�>����=\u0014�B���h�cc(��u��zSe?�<7KH\n" +
				"��\u000B�M��\u0007Q�5��\\4�\u001A\u0002vhO6�;╰���\u0019�4=�?��Im��\"+q\u001CN�\u0013�`���\u001C�pd\u000E\f\u001DxRUlme�\u0014A\u0014��㍖�r���\u0002Ph~���4Qu�́\f\u001Az1����\u0007El=���4xH,���\u0015�\u001B�#.`P�se���\u0015�\u0002�9]\u0011\u0002�a��G\u0005����>�\\�\u0016t�C'1�\u007F�N�\u0013�(��\u0601,�j9PL\u0000\"�H\u0007��\u001E���}����i58��)\u001F#�a6wU\"���Z�\u0019�&\u0014�`5̌_>�\n" +
				"@/V�p\n" +
				"���T\u0002H@��vS)y�vXy��z��\u0000���\u00171�N�\fb��@��e�\\\u000F�VBɰ4)'�ɰB_�*��,Υp���̛� \u007F��>��k;#E�3;v���\u0003��ϵ+q��G�u\u0006(y���Fsui���H�(N^��S)\"�\n" +
				"��\n" +
				"��\u0013�\u0011G$p�M2\\q�V\u001D�2\u0019�\u001D�\u0012ˉ\fy\u0004�́+#�-z��ˍ\u001C�~,��9�\u0011����V\u000B��ن\u0011\u0017\u0017�tV�\t\u0000\u0019\u001D�����\u0007W�\u007F&���(�s�cv�Ҋ4\u001C\u0006}�7\u001C�8���W\u007F(gY_��)��S���Cx(�=\u0004��WQ8�lQ�\\\u0011���OK��\u000F)��\u001B���\u001A���6���6\n" +
				"r\\B��1�\u007F癅��Lf�4�pp\u0005�z�X�O�z\u0018���ɵ��\u000F��#�\u0014�}���\u001D\n" +
				"���\u0018�C%4�%�;u�r��#�鷎UM^U\u0007��E��\u0000%<\"���p�����c�z��)��\u0000o���f�c$\t�\u0010�K��\u0005\u0002�~5W�\n" +
				"9��p��oYt\u0004��>���P��\u007Fb�&-�S���l+}��W�;\u0014�>,\u00034|�\u0003�AV/�7����)i��G�I1�U-�$\u0002�2o�4��G\u0011�_�e��j��%����]'�����Ͼ����0�J/@� �\u001B\n" +
				"\u0015�o^\u008E�\f�\u0007�����\u000F��A*����1�\t\u0007e=V�p�\u001F\u0004h�\u0010�<Z�\u000BO\u001F��Ղ�\u0012�R�?�d*\u001C��!���\u0014\u0007M\u0004�-�g�4~9�pl\u0010��O@�#�E���'6�)\u00002\u0002]�³?����Ã��m\u0001̜��\u0018�U\u0002�6�;*�\\\u0011�n�׃\u0010���%�\t>�e�5?�\u0004�dw#:�\u0007\u0007]F��h\u000F>J\u0017�-��g����\u0004&\u0014���Z�����<줫\u0018O�na�E2�\"�#��B\u001C���!����#\u0007Hu����)ʘ!�le��2ٗ�6h����\u0005\n" +
				";RUa9�(�_�I\u0013��\u0007�'Od�%\u001D!�g�\\\u000EeJ�H�HPH�ayӒ\u0004�'�\u0004��}1g_x����&�\u0001\u000E��-�Z���������>s��/\u0012\f\u001D��M\u000F�ρ�D\u000Fg0�\u0005�E��\u0002\u0004�\u0000��\u0001�\n" +
				"��Z�\u0011i�5\u001E��/�2���.�)ò�\u0004�&\u00056;\u00163�2���/�����O��> EL\u0016�(\u0000k\u0004���Z�`u�\u0018\u000F�\t\\��+��sCx�\u0003b8����}��>�o�\u007F��\u0006���\u000E����\u0006ꢬ�8\u0011���@���5\u0005�Vn0�e�?���Q��\u001A)�\u0005{\n" +
				"\u001D}L���W�K|)\u000F*c#\u001F�?\u001E\u0010W��l���Vݫ�\u0006\u0012���F�\f\"6\f%�W�\u0095����%e/��E�L\t�m\u0019)k���\u0014�\u0012��d1��?\n" +
				"$r��<KԻ6��{��fǀc�%�¤v(Z\\�K\u000F���7\u000F�����T\u0005�\u00045\u007F��\u0001�\u000B\u000F�,ȩ\u0011�\fBt2) k\u0013���ښ�\u000E��-E\u0007��\"�bi\u0017ɵ�\n" +
				"l����������\u0016_a@/Ӛ�K\\m�(\fE5\u0016[���?s�P+<}k�c^jR�#�^�����W,q�꯱�\f�ֱ���R���?9ᱝP��\u0011~G�}�ʀE�`\f�߁Z\"\u000F\u007F��Ƀ�ݳta�\u0015�ˣ�ʝ�\u001F|��\u0019�G\u0005Z6ݗ�鋮�\u007F\u001C\u0015�\u0001uct��6+H0\u0015l��9�L|��q�T\n" +
				"��N\u0006�\u0016\u0004��Z�$\"�\u001C�\u001A�(��/<W@�1�U��w��b�Sg\u0015\n" +
				"��I2�č��y/�0K�n^m�ɨ�\u0019�D\u001F\u000Bw\\>��BK���\u001C��?�~�\u007F\"w�M��}��5$���X/nCy\f\u007F��Rh�%��\t\\`�\\KAtL���\u007FR}1�\u00054�\u0000\u001E���H�t\u0012\u001FEt\u001D_n\u0016���[/�\u000El���5�\u0006�w�\u0004�\u0002\u001A�O��z�]\u001CCj\u0002\\�\u0003V[��2��D��,�K��j�\u007FM\u0005��`��\u0003xޙ���J�zȣ���.����G�\u001E\u0001�QY\u0017�n�0�N}+�ixC����\u001E_?��>�I\u0011�������\"3������\n" +
				"����'�ה�,��?�e��>6\\���\u0013gȳB�|M��s��8����o�l>\u0001���kymM�ؔ)Ӑ�3��\u001DX��Ig\u0005c��\u0014&�{��a��C�Ib�t�8���@W\u0003�ڱ�S\n" +
				"@���\u0015�\u0017��g/�͂�z��!7���&\u0015�\u0014\u000F����\u0013��i���\"ɞ�;��6}O�a.Ct&\u0014\u001D\u0017�\u001D\u0000\\]��Ո*3B\u0015z˽\u007F�\u0004��\f�]Os�d`6�\u0002u�|xG�\uE80EbٲI\u0010�l�Wr~5#K����_�p%�\u001F�g���\u0019�=YB%��� \u0000/�?ٓ�\u0015\u000E��\t�\u000FS���k�\u0016L��fz�*Ф�\u07BFbE\u0016�>�|F\u0014,\n" +
				" �\"��Ӱ\u0011��:]\"��S�WG�\u0000��\u001FI\u0016����\u0002\u0014.\u000E�U��-q(-o07���K\uEA981���x\u0003M�1\u0006�\"Kq�\\�\u001EfO+9\\R��ALj�R\u0002,z/��W�0a\u0012�\u0003N\u0018�\u001Fjq�������fw�\u0011^nD�t�+���[��S�����\u0000\u0015r/Tk��4�\u0004t8Ʀ�\u0005L��\u0016\u0007\u0000�\u0004#��D?\u0005�s\u000F=\u0007-\u0010J\u000E���.�\u000F\u0019<�G�\u007F���$!/͠j�\u000Ff����\u001B4����&\u0007Y[CS��\u0014ד�H\n" +
				"C�.��x�\u000FS�AD�[\u0001m��}4�M$d�=�2\u0014\n" +
				"m�.Rc�W@��l�d1U\u001E�%\n" +
				"�\\\f�(�ܕgMD�-�s�/p\u0019=V�\u0003\u0012\u000Bff��ڤZ{<GPѶ��]L\u0015��ŘOIj���ww�\u001B\u001B���t�}����\u0004\u0010�\u0010JS7Q��_\u0014鰹\u0016\u0007\\�\u0005X�\n" +
				"�wL&�!�F\\�abv�<�a\u0005'�d�\u001E?\u001B\u007F4����j��m��'��5�c�&V�\u000E��$�ˑkg��\u0001+Щc��;<0[���O���k\u001Doϼ��g��+��b�ɜ��:�����x(B�㼐�\u0017y��i��>+��N���\u0014���6J�g�3�a\u0006�����\u0014\u001Ay%�\n" +
				"�\u0001S �\u0006�VA����:�t��3@�ڝ��&�\u001Dc8@�\u0012�\u000F:b{�'У�Z:�2\u0017\u0007����ɼ��:,�2k>J0�\t#v�ګ�\u001DC���]��T\\�l\u000Bs�t\"\u000ES&�T\u0016�\u001BS�c�sjΕ�Dl�t��;&�觶\u0014٘jN��\u0011�\\�����Ae�ϼ\u0001)\u0004�l��US�6S%8\u0001�ETTZk$�xD�\u0014�^TBI7n��Ϸ���S�uo=�\fxY\u001AS�#��nR�\f�Y�\\W�b��r�n\u0002��u��\f�R�N\u0011e�xT�g��\u001D\u0011\u001EYD}�\u0015E���\u001Cd�x.�s�\u0017��\n" +
				"�\\�w�p�s�\u0007�N�\u000F���Y�j�\u0006WiP����0��J�,ޙ\u007F��R.ȿ\u0019�e���Vp�9#�?b�h�m8���\u001E\u000B�*����vy��kT\u0007n��\u0019J\u0001�۟�\u00168�>���ne)�\n" +
				"�6�t���c�\u0000�\tzI�$J[\u0017~��F\uDA40\uDEECY��ٽf��\u0019���\u0015��\u0014��w|��/}�Nj[\u0017��οlA�\n" +
				"��\u000E;��z�\u0001\u001Fd����k�\u00146=էY\n" +
				"�\u001B�o1��@����\u007F�,�Q����W\"9�C����yЫ�\u0011N_�)<<a�-�.,��s��WB8{R��\u001D�\u000B��Z)��\u0016--��W1\u0007��,J<]�VCVM��,�H-\u0011Q��j'���*%T��@��<\u0000�5w^��Tփ��Њ��\u0013k\t�B�Qi b��Y&WP��,\u0017�P\u0012����n4/�^\u000F�\u0015���a\f\"E\u00148.+L\u001B0ha�f�@�8yi�8�G�o��I\u007F�������\u000E��vN�\u0006���=$�\u0017ޠW��֗��n�4���o�0a\u0010e�/3r�\u0001\u000F\u0016\u0019x:<J�p;�\u0000�|�\u001F���2'�\u0006��:~�_�͕5��r\u001Ag\u000Fp[L~�M��0��\u0017��$���=�\"�����\u0018z�֙�2���\u0005��>,W�$Z�ݴ\u001EW�C�\u0018j�\u0006@��y*c�{\u0019T�\u001F�o\u0002yD��9|��PF�vg�\u0019J�,;�d\u07FB�/_v;SW�!�@|��j���ܓ��mμԴz�\u0014u\u0015��\u001A]fѩ\u0012/��%_y�$\u0006�y�a\u001B��z�O^��/\u001A��V���u�����i��\u000F�9G$���-�\u001Dag��uHc���3�X������6����\u0004#W��x9\n" +
				"3�\u0015�!�/u��\u0016��\u000F��%;�\u001CF�y:�����'/�?<O*+rn�Ho\n" +
				"]�;\u0017\u001E\u0010�\u0019�~�d��&\u0002 '�8(:�\u0015�\u0012n��=9\u0012@/H�� ��!���}3.t�$z�'�l)�B���+�M<��:�q��9\u0602�\u0000v�5�����U4�I�v��ܵ|H\u0001\fJf\u0016d\u0003���D!\t��/�;a=����F&\u0002�߹X{���\t\t\u000B̽�{Ɋ�\u001926\u001E���\u007F\t)3K��J�F/��n�;������F��������°B�6\u0005����\u000E�\u001F\u0018cK�2�ɜŢ��/�}u�\n" +
				"5�\u001F��t�f������h�vs|\u0012x��>9\u0001\u0006���E60�\u0004�J���M�%�S��X_�\u0002|��\u007FW��k,(ͪZ�4�l���-�\t_�\u001F��B\u0010��NE��\u0011)?�\u000B�^:�?ԩT�����7rɈ^^uR=q�����a�{��z�R\u001CRWܟ\f\u0001G���\u0001\u001D���\n" +
				"⌔{\f\u0018\u0000��S4�\u0019��g���@\u0002f�\u0012\u0017�\u000F�[�E8�\u000Bă�Jh�%�������Bb˒^i�,���j�؏�m7-�\u007F\u001C�e�CQ��\u0010��Q���:\u0018�,k,.��1 y>��\u001C�Ң�T�*�F�w�n�ͅ���X��ƥ����\u0000E\u000B�{�[��\u0001=�ʁUɇ�[��(�\u007F�\u0003�\\���iR\n" +
				"�dƧ�Q�\u0018��?�}\u0017^G�V�V�yvƦ' ]����C+:���|�6�;\u0012";
		byte[] a = s.getBytes();
		for (int i = 0; i < a.length; i++) {
			System.out.println(i + ":" + a[i]);
		}
		System.out.println(a.length);
	}
}