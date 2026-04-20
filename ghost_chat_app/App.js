import React, { useState, useEffect, useRef } from 'react';
import { 
  StyleSheet, Text, View, TextInput, TouchableOpacity, FlatList, 
  KeyboardAvoidingView, Platform, Image, SafeAreaView, AppState 
} from 'react-native';
import { NavigationContainer } from '@react-navigation/native';
import { createNativeStackNavigator } from '@react-navigation/native-stack';
import { BlurView } from 'expo-blur';
import { LinearGradient } from 'expo-linear-gradient';
import * as ImagePicker from 'expo-image-picker';
import { encode as btoa, decode as atob } from 'base-64';

// Ghost Protocol Constants
const MG = 'GHO:';
const IP = '192.168.4.1'; // ESP32 Static IP

// Core Functions 
function genHash(pwd){
  let s=pwd+"ghost_salt_v5", h1=0xdeadbeef, h2=0x41c6ce57;
  for(let i=0;i<s.length;i++){
    let ch=s.charCodeAt(i);
    h1=Math.imul(h1^ch,2654435761); h2=Math.imul(h2^ch,1597334677);
  }
  h1=Math.imul(h1^(h1>>>16),2246822507)^Math.imul(h2^(h2>>>13),3266489909);
  h2=Math.imul(h2^(h2>>>16),2246822507)^Math.imul(h1^(h1>>>13),3266489909);
  return ((h1>>>0).toString(16).padStart(8,'0')+(h2>>>0).toString(16).padStart(8,'0')).substring(0,8);
}

function enc(t,k){let r='';for(let i=0;i<t.length;i++)r+=String.fromCharCode(t.charCodeAt(i)^k.charCodeAt(i%k.length));return btoa(r);}
function dec(e,k){try{let t=atob(e),r='';for(let i=0;i<t.length;i++)r+=String.fromCharCode(t.charCodeAt(i)^k.charCodeAt(i%k.length));return r;}catch{return'???';}}
const gt=()=>{let d=new Date();return d.getHours().toString().padStart(2,'0')+':'+d.getMinutes().toString().padStart(2,'0')};
function fastHash(s){let h=0;for(let i=0;i<s.length;i++){h=((h<<5)-h)+s.charCodeAt(i);h|=0;}return h;}
function avc(n){let h=0;for(let i=0;i<n.length;i++)h=((h<<5)-h)+n.charCodeAt(i);let c=(h&0xFFFFFF).toString(16).padStart(6,'0');return '#'+c;}

const Stack = createNativeStackNavigator();

// ================= LOGIN SCREEN =================
function LoginScreen({ navigation }) {
  const [name, setName] = useState('');
  const [key, setKey] = useState('');
  const [loading, setLoading] = useState(false);
  const [err, setErr] = useState('');

  const doLogin = async () => {
    if(!name || !key) { setErr('Alias and Key required'); return; }
    setLoading(true); setErr('');
    const roomHash = genHash(key);
    
    try {
      let resp = await fetch(`http://${IP}/login?room=${roomHash}&pass=${encodeURIComponent(key)}&name=${encodeURIComponent(name)}`, { timeout: 3000 });
      let text = await resp.text();
      
      if(text.startsWith('WIPED')){ setErr('Security Wipe Triggered'); }
      else if(text.startsWith('TOK:')){
        let myTok = text.substring(4);
        let jm=MG+'|SYS|'+gt()+'|👤 '+name+' securely joined via Android App';
        fetch(`http://${IP}/send?tok=${myTok}&msg=${encodeURIComponent(enc(jm,key))}`);
        
        navigation.replace('Chat', { name, myKey: key, myTok, roomHash });
      } else { setErr('Invalid Credentials'); }
    } catch(e) {
      setErr('Router unreachable. Connect to Ghost_Net.');
    }
    setLoading(false);
  };

  return (
    <LinearGradient colors={['#0f172a', '#020617']} style={styles.container}>
      <SafeAreaView style={styles.loginCard}>
        <View style={styles.logoCircle}>
          <Text style={{fontSize: 40}}>👻</Text>
        </View>
        <Text style={styles.loginTitle}>Ghost Net Mobile</Text>
        <Text style={styles.loginSub}>The Dark Web of your College</Text>
        
        <View style={styles.inputBox}>
          <TextInput style={styles.input} placeholderTextColor="#64748b" placeholder="Hacker Alias" value={name} onChangeText={setName} />
        </View>
        <View style={styles.inputBox}>
          <TextInput style={styles.input} placeholderTextColor="#64748b" placeholder="Decryption Key" secureTextEntry value={key} onChangeText={setKey} />
        </View>
        
        {err ? <Text style={styles.errorText}>{err}</Text> : null}

        <TouchableOpacity onPress={doLogin} disabled={loading} style={{marginTop: 20}}>
          <LinearGradient colors={['#3b82f6', '#1d4ed8']} style={styles.btn}>
            <Text style={styles.btnText}>{loading ? 'CONNECTING...' : 'INITIALIZE UPLINK'}</Text>
          </LinearGradient>
        </TouchableOpacity>
      </SafeAreaView>
    </LinearGradient>
  );
}

// ================= CHAT SCREEN =================
function ChatScreen({ route, navigation }) {
  const { name, myKey, myTok, roomHash } = route.params;
  const [msg, setMsg] = useState('');
  const [messages, setMessages] = useState([]);
  const [online, setOnline] = useState('0');
  const [lora, setLora] = useState('Offline');
  const pollRef = useRef(null);
  const hashRef = useRef(0);

  const fetchChat = async () => {
    try {
      let r = await fetch(`http://${IP}/read?tok=${myTok}`);
      let raw = await r.text();
      let olp=raw.split('###ONLINE:'), onlineN=olp.length>1?olp[1]:'0';
      setOnline(onlineN);

      let pts=olp[0].split('###TYPE:'), md=pts[0];
      let rawMsgs=md.split('|||').filter(m=>m.length>0);
      
      let parsed = [];
      rawMsgs.forEach((m, idx) => {
        let c=m.startsWith('GHO:|LORA')?m:dec(m,myKey);
        if(!c.startsWith(MG)) return;
        let p=c.split('|'); if(p.length<4) return;
        let snd=p[1],tim=p[2],cnt=p.slice(3).join('|');
        if(cnt.startsWith('CMD:')) return;
        
        parsed.push({
           id: idx.toString(),
           sender: snd,
           time: tim,
           content: cnt,
           isSent: snd === name,
           isSys: snd === 'SYS'
        });
      });
      
      // Simple hash check to avoid pointless re-renders
      let nh = fastHash(parsed.map(x=>x.content).join(''));
      if (nh !== hashRef.current) {
        hashRef.current = nh;
        setMessages(parsed);
      }
    } catch(e) { }
  };

  const checkLora = async () => {
    try {
      let r = await fetch(`http://${IP}/lorastatus`);
      let s = await r.text();
      let pts=s.split(':');
      if(pts[0]==='OK') setLora(pts[1] + ' dBm');
      else setLora('Offline');
    } catch(e){}
  }

  useEffect(() => {
    fetchChat(); checkLora();
    pollRef.current = setInterval(() => { fetchChat(); }, 1500);
    let loraInterval = setInterval(checkLora, 5000);
    return () => { clearInterval(pollRef.current); clearInterval(loraInterval); };
  }, []);

  const sendText = async () => {
    if(!msg.trim()) return;
    let m = msg; setMsg('');
    let raw = MG+'|'+name+'|'+gt()+'|'+m;
    await fetch(`http://${IP}/send?tok=${myTok}&msg=${encodeURIComponent(enc(raw,myKey))}`);
    fetchChat();
  };

  // NATIVE FILE PICKER (Replacing broken web ones)
  const pickImage = async () => {
    let result = await ImagePicker.launchImageLibraryAsync({
      mediaTypes: ImagePicker.MediaTypeOptions.Images,
      allowsEditing: true, quality: 0.5, base64: true,
    });
    if (!result.canceled && result.assets[0]) {
      uploadImage(result.assets[0].uri, result.assets[0].fileName || 'img.jpg');
    }
  };

  const uploadImage = async (uri, fn) => {
    const fd = new FormData();
    fd.append('file', { uri, name: fn, type: 'image/jpeg' });
    try {
      let r = await fetch(`http://${IP}/upload?tok=${myTok}`, { method: 'POST', body: fd });
      if(r.ok) {
         let raw = MG+'|'+name+'|'+gt()+'|IMG:'+fn;
         await fetch(`http://${IP}/send?tok=${myTok}&msg=${encodeURIComponent(enc(raw,myKey))}`);
         fetchChat();
      }
    } catch (e) { alert("Upload failed"); }
  };

  const doLeave = () => {
    let lm=MG+'|SYS|'+gt()+'|👤 '+name+' disconnected Android Node';
    fetch(`http://${IP}/send?tok=${myTok}&msg=${encodeURIComponent(enc(lm,myKey))}`);
    navigation.replace('Login');
  };

  const renderBubble = ({item}) => {
    if (item.isSys) {
      return (
        <View style={styles.sysRow}>
          <View style={styles.sysBubble}><Text style={styles.sysText}>{item.content}</Text></View>
        </View>
      );
    }
    
    let isImg = item.content.startsWith('IMG:');
    let cnt = isImg ? item.content.substring(4) : item.content;
    let color = avc(item.sender);

    return (
      <View style={[styles.msgRow, item.isSent ? styles.msgSent : styles.msgRecv]}>
        {!item.isSent && (
           <View style={[styles.avatar, {backgroundColor: color}]}><Text style={styles.avText}>{item.sender.substring(0,2).toUpperCase()}</Text></View>
        )}
        <View style={styles.bubbleCol}>
           {!item.isSent && <Text style={[styles.senderName, {color}]}>{item.sender}</Text>}
           <LinearGradient 
              colors={item.isSent ? ['#007aff','#0056b3'] : ['#1e293b','#1e293b']} 
              style={[styles.bubble, item.isSent ? styles.bubSent : styles.bubRecv]}
           >
             {isImg ? (
                <Image source={{uri: `http://${IP}/download?name=${cnt}`}} style={styles.chatImg} />
             ) : (
                <Text style={styles.msgText}>{item.content}</Text>
             )}
             <Text style={styles.timeText}>{item.time} {item.isSent && '✓'}</Text>
           </LinearGradient>
        </View>
      </View>
    );
  };

  return (
    <View style={styles.container}>
      <BlurView intensity={80} tint="dark" style={styles.header}>
        <SafeAreaView style={styles.hInner}>
          <TouchableOpacity onPress={doLeave}><Text style={styles.hBtn}>⇦</Text></TouchableOpacity>
          <View style={styles.hCenter}>
            <Text style={styles.hTitle}>Room {roomHash.substring(0,4).toUpperCase()}</Text>
            <Text style={styles.hSub}>LoRa: {lora} | • {online} Online</Text>
          </View>
          <Text style={styles.hBtn}>⚙️</Text>
        </SafeAreaView>
      </BlurView>
      
      <FlatList 
         data={messages} 
         keyExtractor={i=>i.id} 
         renderItem={renderBubble} 
         style={styles.chatList}
         contentContainerStyle={{paddingTop: 110, paddingBottom: 20}}
      />
      
      <KeyboardAvoidingView behavior={Platform.OS === 'ios' ? 'padding' : 'height'}>
         <BlurView intensity={100} tint="dark" style={styles.inputArea}>
            <TouchableOpacity onPress={pickImage} style={styles.fabBtn}><Text style={styles.fabTxt}>+</Text></TouchableOpacity>
            <TextInput 
              style={styles.inputField} 
              placeholderTextColor="#64748b" 
              placeholder="Encrypted payload..." 
              value={msg} 
              onChangeText={setMsg} 
              onSubmitEditing={sendText}
            />
            <TouchableOpacity onPress={sendText} style={styles.sendBtn}><Text style={styles.sendTxt}>➤</Text></TouchableOpacity>
         </BlurView>
      </KeyboardAvoidingView>
    </View>
  );
}

// ================= ROOT NAVIGATOR =================
export default function App() {
  return (
    <NavigationContainer>
      <Stack.Navigator screenOptions={{ headerShown: false }}>
        <Stack.Screen name="Login" component={LoginScreen} />
        <Stack.Screen name="Chat" component={ChatScreen} />
      </Stack.Navigator>
    </NavigationContainer>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: '#020617' },
  loginCard: { flex: 1, justifyContent: 'center', padding: 30 },
  logoCircle: { width: 80, height: 80, backgroundColor: '#1e293b', borderRadius: 24, alignSelf: 'center', justifyContent: 'center', alignItems: 'center', marginBottom: 24, borderWidth: 1, borderColor: 'rgba(255,255,255,0.1)' },
  loginTitle: { fontSize: 32, fontWeight: '900', color: '#fff', textAlign: 'center' },
  loginSub: { fontSize: 14, color: '#94a3b8', textAlign: 'center', marginBottom: 40 },
  inputBox: { backgroundColor: 'rgba(255,255,255,0.05)', borderRadius: 16, marginBottom: 16, borderWidth: 1, borderColor: 'rgba(255,255,255,0.1)' },
  input: { padding: 18, color: '#fff', fontSize: 16 },
  btn: { padding: 18, borderRadius: 16, alignItems: 'center' },
  btnText: { color: '#fff', fontWeight: '800', fontSize: 16, letterSpacing: 1 },
  errorText: { color: '#ef4444', textAlign: 'center', fontWeight: '700', marginBottom: 10 },
  
  header: { position: 'absolute', top: 0, left: 0, right: 0, zIndex: 10, borderBottomWidth: 1, borderBottomColor: 'rgba(255,255,255,0.1)' },
  hInner: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', paddingHorizontal: 20, paddingTop: Platform.OS === 'android' ? 40 : 10, paddingBottom: 15 },
  hTitle: { color: '#fff', fontSize: 18, fontWeight: '800' },
  hSub: { color: '#10b981', fontSize: 12, fontWeight: '700' },
  hCenter: { alignItems: 'center' },
  hBtn: { color: '#fff', fontSize: 24 },
  
  chatList: { flex: 1, paddingHorizontal: 12 },
  msgRow: { flexDirection: 'row', marginVertical: 6, alignItems: 'flex-end' },
  msgSent: { justifyContent: 'flex-end' },
  msgRecv: { justifyContent: 'flex-start' },
  avatar: { width: 30, height: 30, borderRadius: 15, justifyContent: 'center', alignItems: 'center', marginRight: 8 },
  avText: { color: '#fff', fontSize: 12, fontWeight: 'bold' },
  bubbleCol: { maxWidth: '80%' },
  senderName: { fontSize: 12, fontWeight: 'bold', marginBottom: 4, marginLeft: 4 },
  bubble: { padding: 12, borderRadius: 20 },
  bubSent: { borderBottomRightRadius: 4 },
  bubRecv: { borderBottomLeftRadius: 4, borderWidth: 1, borderColor: 'rgba(255,255,255,0.1)' },
  msgText: { color: '#f8fafc', fontSize: 16, lineHeight: 22 },
  timeText: { color: 'rgba(255,255,255,0.5)', fontSize: 10, alignSelf: 'flex-end', marginTop: 4 },
  chatImg: { width: 200, height: 200, borderRadius: 12 },
  
  sysRow: { alignItems: 'center', marginVertical: 10 },
  sysBubble: { backgroundColor: 'rgba(255,255,255,0.1)', paddingHorizontal: 16, paddingVertical: 6, borderRadius: 20 },
  sysText: { color: '#94a3b8', fontSize: 12, fontWeight: 'bold' },
  
  inputArea: { flexDirection: 'row', alignItems: 'center', padding: 12, borderTopWidth: 1, borderTopColor: 'rgba(255,255,255,0.1)' },
  fabBtn: { width: 44, height: 44, borderRadius: 22, backgroundColor: 'rgba(255,255,255,0.1)', justifyContent: 'center', alignItems: 'center', marginRight: 10 },
  fabTxt: { color: '#94a3b8', fontSize: 24, lineHeight: 28 },
  inputField: { flex: 1, backgroundColor: 'rgba(0,0,0,0.5)', borderRadius: 22, paddingHorizontal: 20, height: 44, color: '#fff', fontSize: 16, borderWidth: 1, borderColor: 'rgba(255,255,255,0.1)' },
  sendBtn: { width: 44, height: 44, borderRadius: 22, backgroundColor: '#3b82f6', justifyContent: 'center', alignItems: 'center', marginLeft: 10 },
  sendTxt: { color: '#fff', fontSize: 18 }
});
