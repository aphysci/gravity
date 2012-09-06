import gravity.GravityDataProduct;



public class GravityJavaTest {

    public static void main(String[] argv) {
        System.out.println("in main");

        GravityNode node = new GravityNode();
        GravityReturnCode ret = node.init();
        assert(ret == GravityReturnCode.SUCCESS);
        
        ret = node.registerDataProduct("JavaGDP", 5678, "tcp");
        assert(ret == GravityReturnCode.SUCCESS);
        
        Subscriber s = new Subscriber();
        ret = node.subscribe("JavaGDP", s, "");
        
        ret = node.unregisterDataProduct("JavaGDP");
        assert(ret == GravityReturnCode.SUCCESS);

        ret = node.unregisterDataProduct("JavaGDP");
        assert(ret == GravityReturnCode.REGISTRATION_CONFLICT);
    }
    
    private static class Subscriber implements GravitySubscriber {

		@Override
		public void subscriptionFilled(GravityDataProduct dataProduct) {
			System.out.println("made it to java callback");
			
		}
    	
    }
}

