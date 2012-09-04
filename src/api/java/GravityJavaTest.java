
public class GravityJavaTest {

    static {
        System.loadLibrary("gravity_wrap");
    }

    public static void main(String[] argv) {
        System.out.println("in main");

        GravityNode node = new GravityNode();
        GravityReturnCode ret = node.init();
        assert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.registerDataProduct("JavaGDP", 5678, "tcp");
        assert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.unregisterDataProduct("JavaGDP");
        assert(ret == GravityReturnCode.SUCCESS);

        ret = node.unregisterDataProduct("JavaGDP");
        assert(ret == GravityReturnCode.REGISTRATION_CONFLICT);
    }
}

